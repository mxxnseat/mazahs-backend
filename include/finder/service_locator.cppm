module;

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>

export module service_locator;

export class ServiceLocator {
public:
    template <typename T>
    void register_service(std::shared_ptr<T> service) {
        services[typeid(T)] = service;
    }

    template <typename T, typename... Args>
    void emplace_service(Args&&... args) {
        services[typeid(T)] = std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    T& get() {
        auto it = services.find(typeid(T));
        if (it == services.end()) {
            throw std::runtime_error("Service not found");
        }

        return *std::static_pointer_cast<T>(it->second);
    }

    template <typename T>
    bool contains() const {
        return services.contains(typeid(T));
    };

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> services;
};