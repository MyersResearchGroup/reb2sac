bin_PROGRAMS = reb2sac


# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
reb2sac_LDFLAGS = $(all_libraries)

noinst_HEADERS = \
	abs_phage_lambda2_simulation_run_termination_decider.h	abs_phage_lambda_simulation_run_termination_decider.h	abstraction_engine.h abstraction_method_constructor.h	abstraction_method_manager.h \
				abstraction_method_properties.h	abstraction_reporter.h back_end_processor.h biospice_tsd_printer.h	 \
				bunker_monte_carlo.h common.h	compartment_manager.h reaction_manager.h compiler_def.h \
				confidence_interval_stop_rule.h	critical_concentration_finder.h critical_level_finder.h	critical_level_order_decider.h \
				csv_simulation_printer.h	ctmc_analysis_back_end_processor.h ctmc_analyzer.h	ctmc_stationary_analysis_back_end_processor.h \
				ctmc_stationary_analyzer.h	ctmc_transformation_checker.h \
				default_reb2sac_properties.h	default_simulation_run_termination_decider.h	default_ts_species_level_updater.h dependency_graph.h dll_scope.h	dot_back_end_processor.h \
				ode_simulation.h embedded_runge_kutta_fehlberg_method.h	embedded_runge_kutta_prince_dormand_method.h	emc_leaked_stationary_analyzer.h emc_simulation.h	emc_stationary_analyzer.h \
				euler_method.h \
				flat_phage_lambda2_simulation_run_termination_decider.h	flat_phage_lambda_simulation_run_termination_decider.h	front_end_processor.h 	gillespie_monte_carlo.h monte_carlo.h\
				gnuplot_dat_simulation_printer.h hash_table.h	hse2_back_end_processor.h \
				hse_back_end_processor_common.h	hse_back_end_processor_def.h hse_back_end_processor.h	hse_back_end_processor_util.h \
				hse_logical_statement_handler.h	hse_transformation_checker.h implicit_gear1_method.h	implicit_gear2_method.h \
				implicit_runge_kutta_4_method.h	ir2ctmc_transformer.h ir2xhtml_transformer.h IR.h ir_node.h	kinetic_law_evaluater.h kinetic_law_find_next_time.h kinetic_law_support.h \
				kinetic_law.h law_of_mass_action_util.h	linked_list.h log.h logical_species_node.h \
				marginal_probability_density_evolution_monte_carlo.h markov_analysis_result_reporter.h markov_chain_analysis_properties.h	markov_chain.h \
				nary_level_back_end_process.h nary_order_decider.h	nary_order_transformation_method.h \
				next_reaction_simulation.h	normal_waiting_time_monte_carlo.h null_simulation_printer.h	random_number_generator.h reaction_node.h \
				reb2sac.h	sac_phage_lambda_simulation_run_termination_decider.h	sbml_back_end_processor.h sbml_front_end_processor.h sbml_symtab.h	simulation_method.h \
				simulation_printer.h	simulation_run_termination_decider.h species_critical_level_generator.h	species_critical_level.h \
				species_node.h ssa_with_user_update.h strconv.h	symtab.h tsd_simulation_printer.h \
				ts_species_level_updater.h	type_1_pili1_simulation_run_termination_decider.h	type_1_pili2_simulation_run_termination_decider.h	type1pili_gillespie_ci.h type_1_pili_markov_analysis_result_reporter.h	type.h \
				unit_manager.h function_manager.h constraint_manager.h event_manager.h rule_manager.h util.h	vector.h xhtml_back_end_processor.h \
				analysis_def_parser.tab.h sad_ast.h sad_ast_func_registry.h sad_ast_pretty_printer.h \
	sad_ast_exp_evaluator.h
reb2sac_SOURCES = absolute_activation_inhibition_generation_method.c \
	absolute_inhibition_generation_method.c abs_phage_lambda2_simulation_run_termination_decider.c \
	abs_phage_lambda_simulation_run_termination_decider.c abstraction_engine.c abstraction_method_manager.c \
	abstraction_reporter.c back_end_processor.c birth_death_generation_method2.c \
	birth_death_generation_method3.c birth_death_generation_method4.c birth_death_generation_method5.c \
	birth_death_generation_method6.c birth_death_generation_method7.c birth_death_generation_method.c \
	bunker_monte_carlo.c compartment_manager.c reaction_manager.c \
	confidence_interval_stop_rule.c critical_concentration_finder.c critical_level_finder.c \
	critical_level_order_decider.c csv_simulation_printer.c ctmc_analysis_back_end_processor.c \
	ctmc_analyzer.c ctmc_stationary_analysis_back_end_processor.c \
	ctmc_stationary_analyzer.c ctmc_transformation_checker.c default_reb2sac_properties.c \
	default_simulation_run_termination_decider.c default_ts_species_level_updater.c \
	degradation_stoichiometry_amplifier2.c degradation_stoichiometry_amplifier3.c \
	degradation_stoichiometry_amplifier4.c degradation_stoichiometry_amplifier5.c \
	degradation_stoichiometry_amplifier6.c degradation_stoichiometry_amplifier7.c \
	degradation_stoichiometry_amplifier8.c degradation_stoichiometry_amplifier.c dependency_graph.c \
	dimerization_reduction_level_assignment.c dimerization_reduction_method.c dimer_to_monomer_substitution_method.c \
	dot_back_end_processor.c ode_simulation.c embedded_runge_kutta_fehlberg_method.c \
	embedded_runge_kutta_prince_dormand_method.c emc_leaked_stationary_analyzer.c emc_simulation.c \
	emc_stationary_analyzer.c enzyme_kinetic_qssa_method.c ppta_method.c enzyme_kinetic_rapid_equilibrium_method2.c \
	enzyme_kinetic_rapid_equilibrium_method.c euler_method.c \
	final_state_generation_method.c \
	flat_phage_lambda2_simulation_run_termination_decider.c flat_phage_lambda_simulation_run_termination_decider.c \
	front_end_processor.c gillespie_monte_carlo.c monte_carlo.c\
	gnuplot_dat_simulation_printer.c hash_table.c hse2_back_end_processor.c hse_back_end_processor.c \
	hse_back_end_processor_util.c hse_logical_statement_handler.c hse_transformation_checker.c \
	implicit_gear1_method.c implicit_gear2_method.c implicit_runge_kutta_4_method.c \
	inducer_structure_transformation_method.c ir2ctmc_transformer.c ir2xhtml_transformer.c IR.c ir_node.c \
	irrelevant_species_elimination_method.c kinetic_law.c kinetic_law_constants_simplifier.c \
	kinetic_law_evaluater.c kinetic_law_find_next_time.c kinetic_law_support.c law_of_mass_action_util.c linked_list.c log.c logical_species_node.c \
	main.c marginal_probability_density_evolution_monte_carlo.c markov_analysis_result_reporter.c markov_chain.c \
	max_concentration_reaction_adder.c modifier_constant_propagation_abstraction_method.c \
	modifier_structure_transformation_method.c multiple_products_reaction_elimination_method.c \
	multiple_reactants_reaction_elimination_method.c nary_level_back_end_process2.c nary_level_back_end_process.c \
	nary_order_decider.c nary_order_unary_transformation_method2.c \
	nary_order_unary_transformation_method3.c nary_order_unary_transformation_method.c next_reaction_simulation.c \
	normal_waiting_time_monte_carlo.c null_simulation_printer.c \
	op_site_binding_abstraction_method2.c op_site_binding_abstraction_method3.c \
	op_site_binding_abstraction_method.c pow_kinetic_law_transformer.c random_number_generator.c reaction_node.c \
	reb2sac.c reversible_reaction_structure_transformation_method.c \
	reversible_to_irreversible_transformation_method.c distribute_method.c rnap_operator_binding_abstraction_method.c \
	sac_phage_lambda_simulation_run_termination_decider.c sbml_back_end_processor.c sbml_front_end_processor.c sbml_symtab.c \
	similar_reaction_combining_method.c simulation_printer.c simulation_run_termination_decider.c \
	single_reactant_product_reaction_elimination_method.c species_critical_level.c species_critical_level_generator.c \
	species_node.c ssa_with_user_update.c stoichiometry_amplifier2.c \
	stoichiometry_amplifier3.c stoichiometry_amplifier.c stop_flag_generation_method.c strconv.c \
	symtab.c tsd_simulation_printer.c ts_species_level_updater.c \
	type_1_pili1_simulation_run_termination_decider.c type_1_pili2_simulation_run_termination_decider.c \
	type1pili_gillespie_ci.c type_1_pili_markov_analysis_result_reporter.c \
	unit_manager.c function_manager.c constraint_manager.c event_manager.c rule_manager.c util.c vector.c xhtml_back_end_processor.c \
	analysis_def_parser.tab.c sad_ast.c sad_ast_func_registry.c sad_ast_pretty_printer.c \
	sad_ast_exp_evaluator.c sad_ast_creator.c sad_simulation_run_termination_decider.c \
	analysis_def_scanner.c constraint_simulation_run_termination_decider.c
reb2sac_LDADD = -lgsl -lgslcblas -lsbml

#manually inserted
analysis_def_scanner.c: analysis_def_scanner.l
	flex -oanalysis_def_scanner.c $^

analysis_def_parser.tab.h analysis_def_parser.tab.c: analysis_def_parser.y
	bison -v -d $^

