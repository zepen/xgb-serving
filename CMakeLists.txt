cmake_minimum_required(VERSION 3.8)

project(estimate-server C CXX)

include(common.cmake)

get_filename_component(proto "apis/estimate.proto" ABSOLUTE)
get_filename_component(proto_path "${proto}" PATH)

set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/estimate.pb.cc")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/estimate.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/estimate.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/estimate.grpc.pb.h")
set(service_srcs server/imp/estimator_service_impl.cc server/util.cc)
set(service_hdrs server/imp/estimator_service_impl.h server/util.h)

add_custom_command(
        OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
        DEPENDS "${proto}")

# Include generated *.pb.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "Add grpc/pb include path: ${CMAKE_CURRENT_BINARY_DIR}")

include_directories(${xgboost_INCLUDE_DIRS})
message(STATUS "Add xgboost include path: ${xgboost_INCLUDE_DIRS}")

# grpc_proto
add_library(grpc_proto ${grpc_srcs} ${grpc_hdrs} ${proto_srcs} ${proto_hdrs})
target_link_libraries(grpc_proto ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# service
add_library(service ${service_srcs} ${service_hdrs})
target_link_libraries(service ${_GRPC_GRPCPP} ${xgboost_LIBS})

# Targets greeter_[async_](client|estimator)
add_executable(estimate-server "server/run_server.cc")
target_link_libraries(estimate-server
        grpc_proto
        service
        absl::flags
        absl::flags_parse
        ${xgboost_LIBS}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})