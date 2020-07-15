#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Inherit.h>

namespace vsg
{

    class VSG_DECLSPEC ArrayMapping : public Inherit<Object, ArrayMapping>
    {
    public:
        ArrayMapping();

        const std::string vertex{"vertex"};
        const std::string color{"color"};
        const std::string normal{"nomal"};
        const std::string texcoord{"texcoord"};

        // a classification name per Array used by the shaders
        using Classifications = std::vector<std::string>;
        Classifications classifications;

        std::pair<bool, uint32_t> index(const std::string& name, uint32_t start_location = 0) const
        {
            for(uint32_t location = start_location; location<classifications.size(); ++location)
            {
                if (classifications[location]==name) return {true, location};
            }
            return {false, classifications.size()};
        }

        void read(Input& input) override;
        void write(Output& output) const override;

    protected:
        virtual ~ArrayMapping();

    };

    VSG_type_name(vsg::ArrayMapping);

} // namespace vsg
