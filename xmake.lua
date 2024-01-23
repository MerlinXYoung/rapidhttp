set_xmakever("2.8.5")
set_project("rapidjson")
set_config("plat", os.host())

set_languages("c++11")
set_warnings("all","error")

add_rules("mode.debug", "mode.release")

option("with_pico")
    set_default(false)
    set_showmenu(true)
    set_description("use pico")
option_end()

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
    on_clean(function (target) 
        os.vrun("rm -rf ./include/rapidhttp/layer.hpp")
    end)
    on_build(function (target) 
    --  print(has_config("with_pico"), is_config("with_pico", true), get_config("with_pico") )
        if not os.exists("./include/rapidhttp/layer.hpp") then
            -- print("%%%d: %s", opt.progress, "./include/rapidhttp/layer.hpp")
           
            if has_config("with_pico") then
                print("create rapidhttp/layer.hpp by pico ...")
                os.vrun("./scripts/extract_pico.sh .")
            else 
                print("create rapidhttp/layer.hpp by http_parser ...")
                os.vrun("./scripts/extract_http_parser.sh .")
            end
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
