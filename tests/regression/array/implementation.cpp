/*
 * GridTools
 *
 * Copyright (c) 2014-2019, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <array>

#include <cpp_bindgen/export.hpp>
#include <iostream>

namespace custom_array {
    template <class T>
    struct my_array {
        using data_t = T;

        T *data;
        std::array<int, 3> sizes;
        std::array<int, 3> strides;

        const T &operator()(int i, int j, int k) const {
            assert(i < sizes[0] && j < sizes[1] && k < sizes[2] && "out of bounds");
            return data[i * strides[0] + j * strides[1] + k * strides[2]];
        }

        T &operator()(int i, int j, int k) {
            assert(i < sizes[0] && j < sizes[1] && k < sizes[2] && "out of bounds");
            return data[i * strides[0] + j * strides[1] + k * strides[2]];
        }
    };

    template <typename T>
    my_array<T> bindgen_make_fortran_array_view(bindgen_fortran_array_descriptor *descriptor, my_array<T> *) {
        if (descriptor->rank != 3) {
            throw std::runtime_error("only 3-dimensional arrays are supported");
        }
        return my_array<T>{static_cast<T *>(descriptor->data),
            {descriptor->dims[0], descriptor->dims[1], descriptor->dims[2]},
            {1, descriptor->dims[0], descriptor->dims[0] * descriptor->dims[1]}};
    }

    template <typename T>
    bindgen_fortran_array_descriptor get_fortran_view_meta(my_array<T> *) {
        bindgen_fortran_array_descriptor descriptor;
        descriptor.type = cpp_bindgen::fortran_array_element_kind<T>::value;
        descriptor.rank = 3;
        descriptor.is_acc_present = false;
        return descriptor;
    }

    static_assert(cpp_bindgen::is_fortran_array_bindable<my_array<double>>::value, "");
    static_assert(cpp_bindgen::is_fortran_array_wrappable<my_array<double>>::value, "");
} // namespace custom_array

namespace {
    void fill_array_impl(custom_array::my_array<double> a) {
        for (size_t i = 0; i < a.sizes[0]; ++i)
            for (size_t j = 0; j < a.sizes[1]; ++j)
                for (size_t k = 0; k < a.sizes[2]; ++k) {
                    a(i, j, k) = i * 10000 + j * 100 + k;
                }
    }

    BINDGEN_EXPORT_BINDING_WRAPPED_1(fill_array, fill_array_impl);
} // namespace
