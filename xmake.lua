add_rules("mode.debug", "mode.release")
add_repositories("local-repo deps")
add_requires("stb","glm","vulkan-hpp","vulkan-loader","glfw","mini-test","assimp","spirv-tools","glslang","spirv-cross")
set_languages("c17", "c++20")
add_includedirs("include/","include/res_loader/") 
add_packages("stb","glm","vulkan-hpp","vulkan-loader","glfw","mini-test","assimp","spirv-tools","glslang","spirv-cross")
after_build(function()
    os.cp("$(curdir)/res","$(buildir)")
    print("after_build $(buildir) $(curdir)")
end)

if is_plat("windows") then
    add_defines("PF_WIN32")
end
if is_plat("Android") then
    add_defines("PF_ANDROID")
end

target("vulkan_demo")
    set_kind("binary")
    add_files("src/main.cpp")
    
function example(name)
    target(name)
        set_kind("binary")
        add_files("src/*.cpp|main.cpp","src/Sample/*.cpp","src/Sample/"..name.."/*.cpp","src/ResLoader/*.cpp","src/utils/*.cpp",
        "src/event/*.cpp","src/core/*.cpp","src/comm_comp/*.cpp","src/misc_comp/*.cpp")
end

example("quad")    
example("test_object")
example("instance")
example("depth")
example("fog")
example("cd_rom")

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

