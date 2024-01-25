#pragma once
#include <cstdint>
#include <utility>
#include <vector>

#include "doc.h"
#include "status.h"
#include "util.h"

namespace rapidhttp {

template <class StringT>
struct TResponse : public TDocument<StringT> {
    template <class>
    friend class TParser;
    using base_type = TDocument<StringT>;
    using string_t = typename base_type::string_t;
    using header_type = typename base_type::header_type;
    using headers_type = typename base_type::headers_type;
    using this_type = TResponse<string_t>;
    TResponse() : base_type(HTTP_RESPONSE) {}
    using base_type::base_type;
};

}  // namespace rapidhttp