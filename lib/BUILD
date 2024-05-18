package(default_visibility = ["//visibility:private"])

cc_library(
    name = "automaton",
    hdrs = ["automaton.h"],
)

cc_library(
    name = "dfa",
    srcs = ["dfa.cc"],
    hdrs = ["dfa.h"],
    deps = [
        ":automaton",
    ],
)

cc_library(
    name = "nfa",
    srcs = ["nfa.cc"],
    hdrs = ["nfa.h"],
    deps = [
        ":automaton",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/container:inlined_vector",
    ],
)

cc_library(
    name = "temp",
    srcs = ["temp.cc"],
    hdrs = ["temp.h"],
    deps = [
        ":automaton",
        ":dfa",
        ":nfa",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:inlined_vector",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":automaton",
        ":temp",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "re3",
    srcs = ["re3.cc"],
    hdrs = ["re3.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":parser",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "testing",
    testonly = True,
    srcs = ["testing.cc"],
    hdrs = ["testing.h"],
    deps = [
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_googletest//:gtest",
    ],
    alwayslink = True,
)

cc_test(
    name = "re3_test",
    srcs = ["re3_test.cc"],
    deps = [
        ":parser",
        ":temp",
        ":testing",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
    ],
)
