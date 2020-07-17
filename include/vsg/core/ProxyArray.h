#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>

#include <vsg/maths/mat4.h>
#include <vsg/maths/vec2.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <vsg/io/Input.h>
#include <vsg/io/Output.h>

namespace vsg
{

    template<typename T>
    struct stride_iterator
    {
        using value_type = T;

        std::uint8_t* ptr;
        uint32_t stride;

        stride_iterator& operator++() { ptr += stride; return *this; }
        stride_iterator operator++(int) { stride_iterator reval(*this); ptr += stride; return *this; }
        bool operator==(stride_iterator rhs) const { return ptr == rhs.ptr; }
        bool operator!=(stride_iterator rhs) const { return ptr != rhs.ptr; }

        value_type& operator*() { return *reinterpret_cast<value_type*>(ptr); }
        value_type* operator->() { return reinterpret_cast<value_type*>(ptr); }
    };

    #define VSG_ProxyArray(N, T) \
        using N = ProxyArray<T>; \
        template<>          \
        constexpr const char* type_name<N>() noexcept { return "vsg::" #N; }

    template<typename T>
    class ProxyArray : public Data
    {
    public:
        using value_type = T;

        using iterator = stride_iterator<value_type>;
        using const_iterator = stride_iterator<const value_type>;

        ProxyArray() :
            _size(0),
            _stride(0),
            _data(nullptr) {}

        ProxyArray(ref_ptr<Data> data, std::uint32_t numElements, uint32_t offset, uint32_t stride, Layout layout = Layout())
        {
            assign(data, numElements, offset, stride, layout);
        }

        template<typename... Args>
        static ref_ptr<ProxyArray> create(Args... args)
        {
            return ref_ptr<ProxyArray>(new ProxyArray(args...));
        }

        std::size_t sizeofObject() const noexcept override { return sizeof(ProxyArray); }

        // implementation provided by Visitor.h
        // void accept(Visitor& visitor) override;
        // void accept(ConstVisitor& visitor) const override;

        const char* className() const noexcept override { return type_name<ProxyArray>(); }

        void read(Input& input) override
        {
            Data::read(input);

            uint32_t offset;
            input.read("Size", _size);
            input.read("Stride", _stride);
            input.read("Offset", offset);
            _storage = input.readObject<Data>("Storage");

            if (_storage && _storage->dataPointer()) _data = static_cast<std::uint8_t*>(_storage->dataPointer()) + offset;
            else _data = nullptr;
        }

        void write(Output& output) const override
        {
            std::uint32_t offset = 0;
            if (_storage && _storage->dataPointer()) offset = _data - static_cast<std::uint8_t*>(_storage->dataPointer());

            Data::write(output);
            output.write("Size", _size);
            output.write("Stride", _stride);
            output.write("Offset", offset);
            output.writeObject("Storage", _storage);
        }

        std::size_t size() const { return (_layout.maxNumMipmaps <= 1) ? _size : computeValueCountIncludingMipmaps(_size, 1, 1, _layout.maxNumMipmaps); }

        bool empty() const { return _size == 0; }

        // should ProxyArray be fixed size?
        void clear()
        {
            _size = 0;
            if (_data) { delete[] _data; }
            _data = nullptr;
        }

        void assign(ref_ptr<Data> data, std::uint32_t numElements, uint32_t offset, uint32_t stride, Layout layout = Layout())
        {
            _storage = data;
            _stride = stride;
            _layout = layout;
            if (_storage && _storage->dataPointer())
            {
                _data = static_cast<std::uint8_t*>(_storage->dataPointer()) + offset;
                _size = numElements;
            }
            else
            {
                _data = nullptr;
                _size = 0;
            }
        }

        // release not support on proxy types as data is owneded by the storage data object
        void* dataRelease() override
        {
            _storage = {};
            _data = nullptr;
            _size = 0;
            return nullptr;
        }

        std::size_t valueSize() const override { return sizeof(value_type); }
        std::size_t valueCount() const override { return size(); }

        std::size_t dataSize() const override { return size() * _stride; }

        void* dataPointer() override { return _data; }
        const void* dataPointer() const override { return _data; }

        void* dataPointer(std::size_t i) override { return _data + i*_stride; }
        const void* dataPointer(std::size_t i) const override { return _data + i*_stride; }

        std::uint32_t dimensions() const override { return 1; }

        std::uint32_t width() const override { return _size; }
        std::uint32_t height() const override { return 1; }
        std::uint32_t depth() const override { return 1; }

        value_type* data() { return _data; }
        const value_type* data() const { return _data; }

        value_type& operator[](std::size_t i) { return *(static_cast<value_type*>(_data + i*_stride)); }
        const value_type& operator[](std::size_t i) const  { return *(static_cast<const value_type*>(_data + i*_stride)); }

        value_type& at(std::size_t i) { return *(static_cast<value_type*>(_data + i*_stride)); }
        const value_type& at(std::size_t i) const { return *(static_cast<const value_type*>(_data + i*_stride)); }

        void set(std::size_t i, const value_type& v) { return *(static_cast<value_type*>(_data + i*_stride)) = v; }

        iterator begin() { return iterator{_data, _stride}; }
        const_iterator begin() const { return const_iterator{_data, _stride}; }

        iterator end() { return iterator{_data + _size * _stride, _stride}; }
        const_iterator end() const { return const_iterator{_data + _size * _stride, _stride}; }

    protected:
        virtual ~ProxyArray()
        {
            // memory is held by _storage so will be automatically cleaned up when it's unref'd
        }

    private:
        std::uint32_t _size;
        std::uint32_t _stride;
        std::uint8_t* _data;

        ref_ptr<Data> _storage;
    };

    VSG_ProxyArray(ubyteProxyArray, std::uint8_t);
    VSG_ProxyArray(ushortProxyArray, std::uint16_t);
    VSG_ProxyArray(uintProxyArray, std::uint32_t);
    VSG_ProxyArray(floatProxyArray, float);
    VSG_ProxyArray(doubleProxyArray, double);

    VSG_ProxyArray(vec2ProxyArray, vec2);
    VSG_ProxyArray(vec3ProxyArray, vec3);
    VSG_ProxyArray(vec4ProxyArray, vec4);

    VSG_ProxyArray(dvec2ProxyArray, dvec2);
    VSG_ProxyArray(dvec3ProxyArray, dvec3);
    VSG_ProxyArray(dvec4ProxyArray, dvec4);

    VSG_ProxyArray(bvec2ProxyArray, bvec2);
    VSG_ProxyArray(bvec3ProxyArray, bvec3);
    VSG_ProxyArray(bvec4ProxyArray, bvec4);

    VSG_ProxyArray(ubvec2ProxyArray, ubvec2);
    VSG_ProxyArray(ubvec3ProxyArray, ubvec3);
    VSG_ProxyArray(ubvec4ProxyArray, ubvec4);

    VSG_ProxyArray(svec2ProxyArray, svec2);
    VSG_ProxyArray(svec3ProxyArray, svec3);
    VSG_ProxyArray(svec4ProxyArray, svec4);

    VSG_ProxyArray(usvec2ProxyArray, usvec2);
    VSG_ProxyArray(usvec3ProxyArray, usvec3);
    VSG_ProxyArray(usvec4ProxyArray, usvec4);

    VSG_ProxyArray(ivec2ProxyArray, ivec2);
    VSG_ProxyArray(ivec3ProxyArray, ivec3);
    VSG_ProxyArray(ivec4ProxyArray, ivec4);

    VSG_ProxyArray(uivec2ProxyArray, uivec2);
    VSG_ProxyArray(uivec3ProxyArray, uivec3);
    VSG_ProxyArray(uivec4ProxyArray, uivec4);

    VSG_ProxyArray(mat4ProxyArray, mat4);
    VSG_ProxyArray(dmat4ProxyArray, dmat4);

} // namespace vsg
