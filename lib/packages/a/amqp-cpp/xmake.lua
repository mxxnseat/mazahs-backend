-- AI generated

package("amqp-cpp")
    set_homepage("https://github.com/CopernicaMarketingSoftware/AMQP-CPP")
    set_description("C++ library for communication with RabbitMQ")

    add_urls("git@github.com:CopernicaMarketingSoftware/AMQP-CPP.git")

    add_deps("cmake", "openssl")

    add_configs("shared", {description = "Build shared library", default = false, type = "boolean"})
    add_configs("tcp", {description = "Build TCP module", default = true, type = "boolean"})
    add_configs("examples", {description = "Build examples", default = false, type = "boolean"})

    if is_plat("linux") then
        add_syslinks("pthread", "dl")
    end

    on_install("linux", "macosx", "windows", function (package)
        local configs = {}

        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DAMQP-CPP_BUILD_SHARED=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DAMQP-CPP_BUILD_EXAMPLES=" .. (package:config("examples") and "ON" or "OFF"))

        if package:is_plat("linux") or package:is_plat("macosx") then
            table.insert(configs, "-DAMQP-CPP_LINUX_TCP=" .. (package:config("tcp") and "ON" or "OFF"))
        else
            table.insert(configs, "-DAMQP-CPP_LINUX_TCP=OFF")
        end

        import("package.tools.cmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({
            test = [[
                #include <amqpcpp.h>
                void test() {
                    AMQP::Connection connection(nullptr, AMQP::Login("guest", "guest"), "/");
                }
            ]]
        }, {configs = {languages = "c++17"}}))
    end)