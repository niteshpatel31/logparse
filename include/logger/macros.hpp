#ifndef LOGGER_MACROS_HPP
#define LOGGER_MACROS_HPP

#define LOGGER_TRACE(logger_instance, ...) (logger_instance).trace(__VA_ARGS__)

#define LOGGER_DEBUG(logger_instance, ...) (logger_instance).debug(__VA_ARGS__)

#define LOGGER_INFO(logger_instance, ...) (logger_instance).info(__VA_ARGS__)

#define LOGGER_WARN(logger_instance, ...) (logger_instance).warn(__VA_ARGS__)

#define LOGGER_ERROR(logger_instance, ...) (logger_instance).error(__VA_ARGS__)

#define LOGGER_FATAL(logger_instance, ...) (logger_instance).fatal(__VA_ARGS__)

#endif
