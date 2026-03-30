package("libsndfile")
    set_homepage("https://libsndfile.github.io/libsndfile/")
    set_description("A C library for reading and writing sound files containing sampled audio data.")

    add_urls("https://github.com/libsndfile/libsndfile/archive/refs/tags/$(version).tar.gz",
             "https://github.com/libsndfile/libsndfile.git")

    add_deps("cmake")

    if is_plat("linux", "bsd") then
        add_syslinks("m")
    end

    on_load("windows", "linux", "macosx", "iphoneos", "mingw", "android", function (package)
        if package:config("shared") then
            package:add("deps", "python 3.x", {kind = "binary"})
        end
    end)

    on_install("windows", "linux", "macosx", "iphoneos", "mingw", "android", function (package)
        io.replace("cmake/SndFileChecks.cmake", [[find_package (Ogg 1.3 CONFIG)]], [[]], {plain = true})

        local configs = {
            "-DCMAKE_POLICY_DEFAULT_CMP0057=NEW",
            "-DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON",
            "-DBUILD_PROGRAMS=OFF",
            "-DBUILD_EXAMPLES=OFF",
            "-DBUILD_TESTING=OFF",
            "-DENABLE_MPEG=ON",
            "-DENABLE_EXTERNAL_LIBS=OFF",
            "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"),
            "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release")
        }

        import("package.tools.cmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:has_cfuncs("sf_version_string", {includes = "sndfile.h"}))
    end)