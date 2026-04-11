package("aws-sdk")
    set_homepage("https://github.com/aws/aws-sdk-cpp/tree/main")
    set_description("A c++ AWS SDK")

    add_urls("git@github.com:aws/aws-sdk-cpp.git")

    add_deps("cmake")

    on_load(function (package)
            package:add("frameworks", "CoreFoundation", "Security", "Foundation", "SystemConfiguration", "Network")
    end)

    on_install("windows", "linux", "macosx", "iphoneos", "mingw", "android", function (package)
        local configs = {
            "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"),
            "-DCMAKE_INSTALL_PREFIX=" .. package:installdir(),
            "-DBUILD_ONLY=s3",
            "-DBUILD_SHARED_LIBS=OFF",
            "-DBUILD_DEPS=ON",
            "-DENABLE_TESTING=OFF",
            "-DAUTORUN_UNIT_TESTS=OFF",
            "-DSIMPLE_INSTALL=ON",
            "-DCPP_STANDARD=23",
            "-DCUSTOM_MEMORY_MANAGEMENT=OFF"
        }

        import("package.tools.cmake").install(package, configs)
    end)
