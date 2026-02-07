# if(WIN32)
# add_library (walnut-imgui-part1 SHARED imgui-webgpu-part1.cpp)
# else()
# add_library (walnut-imgui-part1 STATIC imgui-webgpu-part1.cpp)
# endif()
# target_link_libraries(walnut-imgui-part1 PRIVATE imgui::imgui)

add_library (walnut-graphics-webgpu STATIC
                    ImGuiBuildWGPU.cpp
                    WebGPUGraphics.cpp
                    GlfwWebGPURenderingBackend.cpp
                    WebGPUImage.cpp
                    imgui-webgpu-part1.cpp
                )

target_include_directories(walnut-graphics-webgpu PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 
target_link_libraries(walnut-graphics-webgpu PUBLIC glfw3webgpu imgui::imgui glm::glm) 
target_link_directories(walnut-graphics-webgpu PUBLIC $<$<CONFIG:DEBUG>:${WebGPU_LIB_DIRS_DEBUG}>) # TODO: add lib dir for non debug configurations as well

if(WIN32)
    target_link_libraries(walnut-graphics-webgpu PUBLIC
                                # absl_bad_any_cast_impl.lib
                                # absl_bad_optional_access.lib
                                # absl_bad_variant_access.lib
                                # absl_base.lib
                                # absl_city.lib
                                # absl_civil_time.lib
                                # absl_cord.lib
                                # absl_cordz_functions.lib
                                # absl_cordz_handle.lib
                                # absl_cordz_info.lib
                                # absl_cordz_sample_token.lib
                                # absl_cord_internal.lib
                                # absl_crc32c.lib
                                # absl_crc_cord_state.lib
                                # absl_crc_cpu_detect.lib
                                # absl_crc_internal.lib
                                # absl_debugging_internal.lib
                                # absl_demangle_internal.lib
                                # absl_die_if_null.lib
                                # absl_examine_stack.lib
                                # absl_exponential_biased.lib
                                # absl_failure_signal_handler.lib
                                # absl_flags_commandlineflag.lib
                                # absl_flags_commandlineflag_internal.lib
                                # absl_flags_config.lib
                                # absl_flags_internal.lib
                                # absl_flags_marshalling.lib
                                # absl_flags_parse.lib
                                # absl_flags_private_handle_accessor.lib
                                # absl_flags_program_name.lib
                                # absl_flags_reflection.lib
                                # absl_flags_usage.lib
                                # absl_flags_usage_internal.lib
                                # absl_graphcycles_internal.lib
                                # absl_hash.lib
                                # absl_hashtablez_sampler.lib
                                # absl_int128.lib
                                # absl_kernel_timeout_internal.lib
                                # absl_leak_check.lib
                                # absl_log_entry.lib
                                # absl_log_flags.lib
                                # absl_log_globals.lib
                                # absl_log_initialize.lib
                                # absl_log_internal_check_op.lib
                                # absl_log_internal_conditions.lib
                                # absl_log_internal_fnmatch.lib
                                # absl_log_internal_format.lib
                                # absl_log_internal_globals.lib
                                # absl_log_internal_log_sink_set.lib
                                # absl_log_internal_message.lib
                                # absl_log_internal_nullguard.lib
                                # absl_log_internal_proto.lib
                                # absl_log_severity.lib
                                # absl_log_sink.lib
                                # absl_low_level_hash.lib
                                # absl_malloc_internal.lib
                                # absl_periodic_sampler.lib
                                # absl_random_distributions.lib
                                # absl_random_internal_distribution_test_util.lib
                                # absl_random_internal_platform.lib
                                # absl_random_internal_pool_urbg.lib
                                # absl_random_internal_randen.lib
                                # absl_random_internal_randen_hwaes.lib
                                # absl_random_internal_randen_hwaes_impl.lib
                                # absl_random_internal_randen_slow.lib
                                # absl_random_internal_seed_material.lib
                                # absl_random_seed_gen_exception.lib
                                # absl_random_seed_sequences.lib
                                # absl_raw_hash_set.lib
                                # absl_raw_logging_internal.lib
                                # absl_scoped_set_env.lib
                                # absl_spinlock_wait.lib
                                # absl_stacktrace.lib
                                # absl_status.lib
                                # absl_statusor.lib
                                # absl_strerror.lib
                                # absl_strings.lib
                                # absl_strings_internal.lib
                                # absl_string_view.lib
                                # absl_str_format_internal.lib
                                # absl_symbolize.lib
                                # absl_synchronization.lib
                                # absl_throw_delegate.lib
                                # absl_time.lib
                                # absl_time_zone.lib
                                # absl_vlog_config_internal.lib
                                # webgpu_dawn.lib
                                # dawn_common.lib
                                # dawn_native.lib
                                # dawn_platform.lib
                                # dawn_proc.lib
                                # dawn_utils.lib
                                # dawn_wire.lib
                                # SPIRV-Tools-opt.lib
                                # SPIRV-Tools.lib
                                # tint_api.lib
                                # tint_api_common.lib
                                # tint_lang_core.lib
                                # tint_lang_core_common.lib
                                # tint_lang_core_constant.lib
                                # tint_lang_core_intrinsic.lib
                                # tint_lang_core_ir.lib
                                # tint_lang_core_ir_transform.lib
                                # tint_lang_core_ir_transform_common.lib
                                # tint_lang_core_type.lib
                                # tint_lang_hlsl_writer_common.lib
                                # tint_lang_spirv.lib
                                # tint_lang_spirv_intrinsic.lib
                                # tint_lang_spirv_ir.lib
                                # tint_lang_spirv_type.lib
                                # tint_lang_spirv_writer.lib
                                # tint_lang_spirv_writer_ast_printer.lib
                                # tint_lang_spirv_writer_ast_raise.lib
                                # tint_lang_spirv_writer_common.lib
                                # tint_lang_spirv_writer_printer.lib
                                # tint_lang_spirv_writer_raise.lib
                                # tint_lang_wgsl.lib
                                # tint_lang_wgsl_ast.lib
                                # tint_lang_wgsl_ast_transform.lib
                                # tint_lang_wgsl_common.lib
                                # tint_lang_wgsl_features.lib
                                # tint_lang_wgsl_helpers.lib
                                # tint_lang_wgsl_inspector.lib
                                # tint_lang_wgsl_intrinsic.lib
                                # tint_lang_wgsl_ir.lib
                                # tint_lang_wgsl_program.lib
                                # tint_lang_wgsl_reader.lib
                                # tint_lang_wgsl_reader_lower.lib
                                # tint_lang_wgsl_reader_parser.lib
                                # tint_lang_wgsl_reader_program_to_ir.lib
                                # tint_lang_wgsl_resolver.lib
                                # tint_lang_wgsl_sem.lib
                                # tint_lang_wgsl_writer.lib
                                # tint_lang_wgsl_writer_ast_printer.lib
                                # tint_lang_wgsl_writer_ir_to_program.lib
                                # tint_lang_wgsl_writer_raise.lib
                                # tint_lang_wgsl_writer_syntax_tree_printer.lib
                                # tint_utils_containers.lib
                                # tint_utils_debug.lib
                                # tint_utils_diagnostic.lib
                                # tint_utils_generator.lib
                                # tint_utils_ice.lib
                                # tint_utils_id.lib
                                # tint_utils_macros.lib
                                # tint_utils_math.lib
                                # tint_utils_memory.lib
                                # tint_utils_reflection.lib
                                # tint_utils_result.lib
                                # tint_utils_rtti.lib
                                # tint_utils_strconv.lib
                                # tint_utils_symbol.lib
                                # tint_utils_system.lib
                                # tint_utils_text.lib
                                # tint_utils_traits.lib
                                webgpu
                        )
else()
    target_link_libraries(walnut-graphics-webgpu PUBLIC glfw3webgpu imgui::imgui glm::glm
                                absl_bad_any_cast_impl.a
                                absl_bad_optional_access.a
                                absl_bad_variant_access.a
                                absl_base.a
                                absl_city.a
                                absl_civil_time.a
                                absl_cord.a
                                absl_cordz_functions.a
                                absl_cordz_handle.a
                                absl_cordz_info.a
                                absl_cordz_sample_token.a
                                absl_cord_internal.a
                                absl_crc32c.a
                                absl_crc_cord_state.a
                                absl_crc_cpu_detect.a
                                absl_crc_internal.a
                                absl_debugging_internal.a
                                absl_demangle_internal.a
                                absl_die_if_null.a
                                absl_examine_stack.a
                                absl_exponential_biased.a
                                absl_failure_signal_handler.a
                                absl_flags_commandlineflag.a
                                absl_flags_commandlineflag_internal.a
                                absl_flags_config.a
                                absl_flags_internal.a
                                absl_flags_marshalling.a
                                absl_flags_parse.a
                                absl_flags_private_handle_accessor.a
                                absl_flags_program_name.a
                                absl_flags_reflection.a
                                absl_flags_usage.a
                                absl_flags_usage_internal.a
                                absl_graphcycles_internal.a
                                absl_hash.a
                                absl_hashtablez_sampler.a
                                absl_int128.a
                                absl_kernel_timeout_internal.a
                                absl_leak_check.a
                                absl_log_entry.a
                                absl_log_flags.a
                                absl_log_globals.a
                                absl_log_initialize.a
                                absl_log_internal_check_op.a
                                absl_log_internal_conditions.a
                                absl_log_internal_fnmatch.a
                                absl_log_internal_format.a
                                absl_log_internal_globals.a
                                absl_log_internal_log_sink_set.a
                                absl_log_internal_message.a
                                absl_log_internal_nullguard.a
                                absl_log_internal_proto.a
                                absl_log_severity.a
                                absl_log_sink.a
                                absl_low_level_hash.a
                                absl_malloc_internal.a
                                absl_periodic_sampler.a
                                absl_random_distributions.a
                                absl_random_internal_distribution_test_util.a
                                absl_random_internal_platform.a
                                absl_random_internal_pool_urbg.a
                                absl_random_internal_randen.a
                                absl_random_internal_randen_hwaes.a
                                absl_random_internal_randen_hwaes_impl.a
                                absl_random_internal_randen_slow.a
                                absl_random_internal_seed_material.a
                                absl_random_seed_gen_exception.a
                                absl_random_seed_sequences.a
                                absl_raw_hash_set.a
                                absl_raw_logging_internal.a
                                absl_scoped_set_env.a
                                absl_spinlock_wait.a
                                absl_stacktrace.a
                                absl_status.a
                                absl_statusor.a
                                absl_strerror.a
                                absl_strings.a
                                absl_strings_internal.a
                                absl_string_view.a
                                absl_str_format_internal.a
                                absl_symbolize.a
                                absl_synchronization.a
                                absl_throw_delegate.a
                                absl_time.a
                                absl_time_zone.a
                                absl_vlog_config_internal.a
                                webgpu_dawn.a
                                dawn_common.a
                                dawn_native.a
                                dawn_platform.a
                                dawn_proc.a
                                dawn_utils.a
                                dawn_wire.a
                                SPIRV-Tools-opt.a
                                SPIRV-Tools.a
                                tint_api.a
                                tint_api_common.a
                                tint_lang_core.a
                                tint_lang_core_common.a
                                tint_lang_core_constant.a
                                tint_lang_core_intrinsic.a
                                tint_lang_core_ir.a
                                tint_lang_core_ir_transform.a
                                tint_lang_core_ir_transform_common.a
                                tint_lang_core_type.a
                                tint_lang_hlsl_writer_common.a
                                libtint_lang_msl.a
                                libtint_lang_msl_intrinsic.a
                                libtint_lang_msl_ir.a
                                libtint_lang_msl_writer.a
                                libtint_lang_msl_writer_ast_printer.a
                                libtint_lang_msl_writer_ast_raise.a
                                libtint_lang_msl_writer_common.a
                                libtint_lang_msl_writer_printer.a
                                libtint_lang_msl_writer_raise.a
                                tint_lang_wgsl.a
                                tint_lang_wgsl_ast.a
                                tint_lang_wgsl_ast_transform.a
                                tint_lang_wgsl_common.a
                                tint_lang_wgsl_features.a
                                tint_lang_wgsl_helpers.a
                                tint_lang_wgsl_inspector.a
                                tint_lang_wgsl_intrinsic.a
                                tint_lang_wgsl_ir.a
                                tint_lang_wgsl_program.a
                                tint_lang_wgsl_reader.a
                                tint_lang_wgsl_reader_lower.a
                                tint_lang_wgsl_reader_parser.a
                                tint_lang_wgsl_reader_program_to_ir.a
                                tint_lang_wgsl_resolver.a
                                tint_lang_wgsl_sem.a
                                tint_lang_wgsl_writer.a
                                tint_lang_wgsl_writer_ast_printer.a
                                tint_lang_wgsl_writer_ir_to_program.a
                                tint_lang_wgsl_writer_raise.a
                                tint_lang_wgsl_writer_syntax_tree_printer.a
                                tint_utils_containers.a
                                tint_utils_debug.a
                                tint_utils_diagnostic.a
                                tint_utils_generator.a
                                tint_utils_ice.a
                                tint_utils_id.a
                                tint_utils_macros.a
                                tint_utils_math.a
                                tint_utils_memory.a
                                tint_utils_reflection.a
                                tint_utils_result.a
                                tint_utils_rtti.a
                                tint_utils_strconv.a
                                tint_utils_symbol.a
                                tint_utils_system.a
                                tint_utils_text.a
                                tint_utils_traits.a
                        )               
endif()
