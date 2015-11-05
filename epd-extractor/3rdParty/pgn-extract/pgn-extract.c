//
// Created by Evgenii on 02.11.2015.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bool.h"
#include "mymalloc.h"
#include "defs.h"
#include "typedef.h"
#include "tokens.h"
#include "taglist.h"
#include "lex.h"
#include "moves.h"
#include "map.h"
#include "lists.h"
#include "output.h"
#include "end.h"
#include "grammar.h"
#include "hashing.h"
#include "argsfile.h"
#include "pgn-extract.h"

/* The maximum length of an output line.  This is conservatively
 * slightly smaller than the PGN export standard of 80.
 */
#define MAX_LINE_LENGTH 75

/* Define a file name relative to the current directory representing
 * a file of ECO classificiations.
 */
#ifndef DEFAULT_ECO_FILE
#define DEFAULT_ECO_FILE "eco.pgn"
#endif

/* This structure holds details of the program state
 * available to all parts of the program.
 * This goes against the grain of good structured programming
 * principles, but most of these fields are set from the program's
 * arguments and are read-only thereafter. If I had done this in
 * C++ there would have been a cleaner interface!
 */
StateInfo GlobalState = {FALSE,                /* skipping_current_game */
                         FALSE,                /* check_only (-r) */
                         TRUE,                 /* verbose (-s) */
                         TRUE,                 /* keep_NAGs (-N) */
                         TRUE,                 /* keep_comments (-C) */
                         TRUE,                 /* keep_variations (-V) */
                         ALL_TAGS,             /* tag_output_form (-7, --notags) */
                         TRUE,                 /* match_permutations (-v) */
                         FALSE,                /* positional_variations (-x) */
                         FALSE,                /* use_soundex (-S) */
                         FALSE,                /* suppress_duplicates (-D) */
                         FALSE,                /* suppress_originals (-U) */
                         FALSE,          /* fuzzy_match_duplicates (--fuzzy) */
                         0,              /* fuzzy_match_depth (--fuzzy) */
                         FALSE,                /* check_tags */
                         FALSE,                /* add_ECO (-e) */
                         FALSE,                /* parsing_ECO_file (-e) */
                         DONT_DIVIDE,          /* ECO_level (-E) */
                         SAN,                  /* output_format (-W) */
                         MAX_LINE_LENGTH,      /* max_line_length (-w) */
                         FALSE,                /* use_virtual_hash_table (-Z) */
                         FALSE,                /* check_move_bounds (-b) */
                         FALSE,                /* match_only_checkmate (-M) */
                         FALSE,                /* match_only_stalemate (--stalemate) */
                         TRUE,                 /* keep_move_numbers (--nomovenumbers) */
                         TRUE,                 /* keep_results (--noresults) */
                         TRUE,                 /* keep_checks (--nochecks) */
                         FALSE,                /* output_evaluation (--evaluation) */
                         FALSE,                /* keep_broken_games (--keepbroken) */
                         FALSE,                /* suppress_redundant_ep_info (--nofauxep) */
                         0,                    /* depth_of_positional_search */
                         0,                    /* num_games_processed */
                         0,                    /* num_games_matched */
                         0,                    /* games_per_file (-#) */
                         1,                    /* next_file_number */
                         0,                    /* lower_move_bound */
                         10000,                /* upper_move_bound */
                         -1,                   /* output_ply_limit (--plylimit) */
                         0,                    /* matching_game_number */
                         FALSE,                /* output_FEN_string */
                         FALSE,                /* add_FEN_comments (--fencomments) */
                         FALSE,                /* add_position_match_comments (--markmatches) */
                         FALSE,                /* output_total_plycount (--totalplycount) */
                         FALSE,                /* add_hashcode_tag (--addhashcode) */
                         "MATCH",          /* position_match_comment (--markpositionmatches) */
                         (char *) NULL,         /* current_input_file */
                         NORMALFILE,           /* current_file_type */
                         DEFAULT_ECO_FILE,     /* eco_file (-e) */
                         (FILE *) NULL,         /* outputfile (-o, -a). Default is stdout */
                         (char *) NULL,         /* output_filename (-o, -a) */
                         (FILE *) NULL,         /* logfile (-l). Default is stderr */
                         (FILE *) NULL,         /* duplicate_file (-d) */
                         (FILE *) NULL,         /* non_matching_file (-n) */
};

void *divide(const char *input, const char *output[]) {

	reset_list_of_files(); //added a function to clear file list
	GlobalState.logfile = stderr;
	set_output_line_length(MAX_LINE_LENGTH);
	init_game_header();
	init_tag_lists();
	init_hashtab();
	init_lex_tables();
	add_filename_to_source_list(input, NORMALFILE);

	GlobalState.verbose = FALSE;
	GlobalState.add_FEN_comments = FALSE;
	GlobalState.tag_output_format = ALL_TAGS;
	GlobalState.keep_results = TRUE;
	GlobalState.keep_comments = FALSE;
	GlobalState.suppress_duplicates = TRUE;
	char *results[3] = {"1-0", "0-1", "1/2-1/2"};

	for (int i = 0; i < 3; ++i) {
		GlobalState.output_filename = output[i];
		GlobalState.outputfile = must_open_file(output[i], "w");

		init_tag_lists();
		add_tag_to_list(RESULT_TAG, results[i], NONE);


		init_duplicate_hash_table();
		if (!open_first_file()) {
			exit(1);
		}
		yyparse(GlobalState.current_file_type);
/* Remove any temporary files. */
		clear_duplicate_hash_table();

	}
}


void *extract(const char *input, const char *output) {

	reset_list_of_files();
	GlobalState.logfile = stderr;
	set_output_line_length(MAX_LINE_LENGTH);
	init_game_header();
	init_tag_lists();
	init_hashtab();
	init_lex_tables();
	add_filename_to_source_list(input, NORMALFILE);
	GlobalState.output_filename = output;
	GlobalState.outputfile = must_open_file(output, "w");
	GlobalState.add_FEN_comments = TRUE;
	GlobalState.tag_output_format = NO_TAGS;
	GlobalState.keep_results = FALSE;
	GlobalState.keep_comments = TRUE;
	GlobalState.suppress_duplicates = TRUE;


	init_duplicate_hash_table();
	if (!open_first_file()) {
		exit(1);
	}
	yyparse(GlobalState.current_file_type);
/* Remove any temporary files. */
	clear_duplicate_hash_table();
	fclose(GlobalState.outputfile);
	remove(input);
}