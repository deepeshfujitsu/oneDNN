/*******************************************************************************
* Copyright 2020-2024 Intel Corporation
* Copyright 2020 Codeplay Software Limited
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef GPU_NVIDIA_STREAM_HPP
#define GPU_NVIDIA_STREAM_HPP

#include <cuda.h>
#include <cudnn.h>
#include <cublas_v2.h>

#include "common/engine.hpp"

#include "xpu/sycl/stream_impl.hpp"

#include "gpu/gpu_stream.hpp"

namespace dnnl {
namespace impl {
namespace gpu {
namespace nvidia {

class stream_t : public gpu::stream_t {
public:
    cublasHandle_t &get_cublas_handle(CUstream cuda_stream = nullptr);
    cudnnHandle_t &get_cudnn_handle(CUstream cuda_stream = nullptr);

    static status_t create_stream(impl::stream_t **stream,
            impl::engine_t *engine, impl::stream_impl_t *stream_impl) {
        std::unique_ptr<nvidia::stream_t> s(
                new nvidia::stream_t(engine, stream_impl));
        if (!s) return status::out_of_memory;

        status_t status = s->init();
        if (status != status::success) {
            // Stream owns stream_impl only if it's created successfully (including initialization).
            s->impl_.release();
            return status;
        }

        *stream = s.release();
        return status::success;
    }

    ::sycl::queue &queue() const { return *impl()->queue(); }

    status_t wait() override {
        queue().wait_and_throw();
        return status::success;
    }

    status_t copy(const memory_storage_t &src, const memory_storage_t &dst,
            size_t size, const xpu::event_t &deps,
            xpu::event_t &out_dep) override {
        CHECK(impl()->copy(this, src, dst, size, deps, out_dep));

        if (is_profiling_enabled()) {
            assert(impl::sycl::sycl_event_t::from(out_dep).size() == 1);
            auto sycl_event = utils::make_unique<impl::sycl::sycl_event_t>(
                    std::vector<::sycl::event> {
                            impl::sycl::sycl_event_t::from(out_dep)[0]});
            profiler_->register_event(std::move(sycl_event));
        }
        return status::success;
    }

    status_t fill(const memory_storage_t &dst, uint8_t pattern, size_t size,
            const xpu::event_t &deps, xpu::event_t &out_dep) override {
        CHECK(impl()->fill(dst, pattern, size, deps, out_dep));

        if (is_profiling_enabled()) {
            assert(impl::sycl::sycl_event_t::from(out_dep).size() == 1);
            auto sycl_event = utils::make_unique<impl::sycl::sycl_event_t>(
                    std::vector<::sycl::event> {
                            impl::sycl::sycl_event_t::from(out_dep)[0]});
            profiler_->register_event(std::move(sycl_event));
        }
        return status::success;
    }

    const impl::sycl::sycl_context_t &sycl_ctx() const {
        return impl()->sycl_ctx();
    }
    impl::sycl::sycl_context_t &sycl_ctx() { return impl()->sycl_ctx(); }

    xpu::context_t &ctx() override { return impl()->sycl_ctx(); }
    const xpu::context_t &ctx() const override { return impl()->sycl_ctx(); }

    ::sycl::event get_output_event() const {
        return impl()->get_output_event();
    }

    void register_deps(::sycl::handler &cgh) const {
        return impl()->register_deps(cgh);
    }

    status_t interop_task(std::function<void(::sycl::handler &)>);
    CUstream get_underlying_stream();
    CUcontext get_underlying_context();
    CUdevice get_underlying_device();

private:
    xpu::sycl::stream_impl_t *impl() const {
        return (xpu::sycl::stream_impl_t *)stream_t::impl_.get();
    }

    status_t init();
    stream_t(impl::engine_t *engine, impl::stream_impl_t *stream_impl)
        : gpu::stream_t(engine, stream_impl) {}
};

} // namespace nvidia
} // namespace gpu
} // namespace impl
} // namespace dnnl

#endif
