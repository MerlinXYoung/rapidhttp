set_xmakever("2.8.5")
set_project("rapidjson")
set_config("plat", os.host())

set_languages("c++11")
set_warnings("all","error")

add_rules("mode.debug", "mode.release")

add_requireconfs("gtest", {configs={main=true}})
add_requires("gtest 1.12.0", "benchmark")

add_includedirs("./include")

target("rapidjson")
    set_kind("headeronly")
    on_config(function (target) 
        if not os.exists("./third_party/http-parser/http_parser.h") then 
            os.vrun("git submodule update --init --force")
        end
    end)
    on_build(function (target) 
        if not os.exists("./include/rapidhttp/layer.hpp") then
            os.vrun("./scripts/extract_http_parser.sh .")
        end
    end)


function scan_targets(prefix)
    for _, x in ipairs(os.files(prefix.."/*.cpp")) do 
        local s = path.filename(x)
        local tar = s:sub(1, #s - 4)       -- target
        local source = path.relative(x, ".")  -- source
        target(tar)
            set_kind("binary")
            set_default(false)
            add_files(source)
            add_deps("rapidjson")
            add_packages("benchmark")
        target_end()
    end
end

scan_targets("benchmark")
scan_targets("tutorial")

target("unitest")
    set_kind("binary")
    set_default(false)
    add_files("test/*.cpp")
    add_deps("rapidjson")
    add_packages("gtest")
