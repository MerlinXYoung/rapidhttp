set_xmakever("2.8.5")
set_project("rapidjson")
set_config("plat", os.host())

set_languages("c++11")
set_warnings("all","error")

add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_configvar("USE_PICO", 0)
option("with_pico")
    set_default(false)
    set_showmenu(true)
    set_description("use pico")
    set_configvar("USE_PICO", 1)
option_end()

option("WITH_PROFILE")
    set_default(false)
    set_showmenu(true)
    set_description("link benchmark with profiler")
    add_defines("PROFILE=1","_GNU_SOURCE=1")

    add_cxflags("-MD", "-g")
    add_cxflags("-MT", {force=true})
    
    
    -- add_links("tcmalloc_minimal","profiler", "unwind")
    -- target_link_libraries(benchmark -lprofiler -lunwind)
option_end()

add_requireconfs("gtest", {configs={main=true}})
add_requireconfs("gperftools", {configs={unwind=true}})
add_requires("gtest 1.12.0", "benchmark", "gperftools")
-- , "libunwind")

add_includedirs("./include")

target("rapidjson")
    set_kind("headeronly")
    add_headerfiles("./include/**.h","./include/**.hpp", {prefixdir = "rapidhttp"})
    -- add_installfiles("./include/**.h", {prefixdir = "rapidhttp"})
    set_configdir("./include/rapidhttp/")
    add_configfiles("./include/rapidhttp/cmake_config.h.in",{filename = "cmake_config.h"})
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
    -- on_install(function (target)
    --     os.vrun("mkdir "..target:installdir().."/include/rapidhttp")
    -- end)



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
        target_end()
    end
end

-- scan_targets("benchmark")
scan_targets("tutorial")

target("unitest")
    set_kind("binary")
    set_default(false)
    add_files("test/*.cpp")
    add_deps("rapidjson")
    add_packages("gtest")

    add_tests("default")
-- print("WITH_PROFILE:",has_config("WITH_PROFILE"))
target("benchmark")
    set_kind("binary")
    set_default(false)
    add_files("benchmark/*.cpp")
    add_deps("rapidjson")
    add_options("WITH_PROFILE")
    set_symbols("debug")
    
    -- add_cxflags("-g")
    if has_config("WITH_PROFILE") then
        add_packages("gperftools", {links={"profiler","unwind"}})
        -- add_packages("libunwind")
        
    end
    add_packages("benchmark",{links={"benchmark", "tcmalloc_minimal", "pthread"}}) 

    -- before_link(function (target)
    --      print("ldflags:",target:get("ldflags"))
    --      print("cxflags:",target:get("cxflags"))
    --      print("links:", target:get("links"))
    --     --  print("taraget:", target)
        
    -- end)
    -- on_link(function (target) 
        
    -- end)
    
target_end()
    
