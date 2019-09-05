/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file var_type_def_st.h
 * @brief Structure declarations for typedvar type definitions.
 *
 * This structure is used for defining new variable types.  If you are not
 * defining a new variable type for use by the configuration management
 * system, you don't need this structure.
 *
 * For defining new variables, see the types in conftypes.h.
 *
 * For data-driven access to configuration variables, see the other members of
 * lib/confmgt/.
 *
 * STATUS NOTE: It is not yet possible to actually define new variables
 *   outside of config.c, and many of the types that will eventually be used
 *   to do so are not yet moved. This will change as more of #29211 is
 *   completed.
 **/

#ifndef TOR_LIB_CONFMGT_VAR_TYPE_DEF_ST_H
#define TOR_LIB_CONFMGT_VAR_TYPE_DEF_ST_H

#include <stdbool.h>

struct config_line_t;

/**
 * A structure full of functions pointers to implement a variable type.
 *
 * Every type MUST implement parse or kv_parse and encode or kv_encode;
 * the other functions pointers MAY be NULL.
 *
 * All functions here take a <b>params</b> argument, whose value
 * is determined by the type definition.  Two types may have the
 * same functions, but differ only in parameters.
 **/
struct var_type_fns_t {
  /**
   * Try to parse a string in <b>value</b> that encodes an object of this
   * type.  On success, adjust the lvalue pointed to by <b>target</b> to hold
   * that value, and return 0.  On failure, set *<b>errmsg</b> to a newly
   * allocated string holding an error message, and return -1.
   **/
  int (*parse)(void *target, const char *value, char **errmsg,
               const void *params);
  /**
   * Try to parse a single line from the head of<b>line</b> that encodes
   * an object of this type.  On success and failure, behave as in the parse()
   * function.
   *
   * If this function is absent, it is implemented in terms of parse().
   *
   * All types for which keys are significant should use this method. For
   * example, a "linelist" type records the actual keys that are given
   * for each line, and so should use this method.
   *
   * Note that although multiple lines may be provided in <b>line</b>,
   * only the first one should be handled by this function.
   **/
  int (*kv_parse)(void *target, const struct config_line_t *line,
                  char **errmsg, const void *params);
  /**
   * Encode a value pointed to by <b>value</b> and return its result
   * in a newly allocated string.  The string may need to be escaped.
   *
   * If this function is absent, it is implemented in terms of kv_encode().
   *
   * Returns NULL if this option has a NULL value, or on internal error.
   *
   * Requirement: all strings generated by encode() should produce a
   * semantically equivalent value when given to parse().
   **/
  char *(*encode)(const void *value, const void *params);
  /**
   * As encode(), but returns a newly allocated config_line_t object.  The
   * provided <b>key</b> is used as the key of the lines, unless the type is
   * one that encodes its own keys.
   *
   * Unlike kv_parse(), this function will return a list of multiple lines,
   * if <b>value</b> is such that it must be encoded by multiple lines.
   *
   * Returns NULL if there are no lines to encode, or on internal error.
   *
   * If this function is absent, it is implemented in terms of encode().
   **/
  struct config_line_t *(*kv_encode)(const char *key, const void *value,
                                     const void *params);
  /**
   * Free all storage held in <b>arg</b>, and set <b>arg</b> to a default
   * value -- usually zero or NULL.
   *
   * If this function is absent, the default implementation does nothing.
   **/
  void (*clear)(void *arg, const void *params);
  /**
   * Return true if <b>a</b> and <b>b</b> hold the same value, and false
   * otherwise.
   *
   * If this function is absent, it is implemented by encoding both a and
   * b and comparing their encoded strings for equality.
   **/
  bool (*eq)(const void *a, const void *b, const void *params);
  /**
   * Try to copy the value from <b>value</b> into <b>target</b>.
   * On success return 0; on failure return -1.
   *
   * If this function is absent, it is implemented by encoding the value
   * into a string, and then parsing it into the target.
   **/
  int (*copy)(void *target, const void *value, const void *params);
  /**
   * Check whether <b>value</b> holds a valid value according to the
   * rules of this type; return true if it does and false if it doesn't.
   *
   * The default implementation for this function assumes that all
   * values are valid.
   **/
  bool (*ok)(const void *value, const void *params);
  /**
   * Mark a value of this variable as "fragile", so that future attempts to
   * assign to this variable will replace rather than extending it.
   *
   * The default implementation for this function does nothing.
   *
   * Only meaningful for types with is_cumulative set.
   **/
  void (*mark_fragile)(void *value, const void *params);
};

/**
 * Flag for var_type_def_t.
 * Set iff a variable of this type can never be set directly by name.
 **/
#define VTFLAG_UNSETTABLE (1u<<0)
/**
 * Flag for var_type_def_t.
 * Set iff a variable of this type is always contained in another
 * variable, and as such doesn't need to be dumped or copied
 * independently.
 **/
#define VTFLAG_CONTAINED (1u<<1)
/**
 * Flag for var_type_def_t.
 * Set iff a variable of this type can be set more than once without
 * destroying older values. Such variables should implement "mark_fragile".
 */
#define VTFLAG_CUMULATIVE (1u<<2)

/**
 * A structure describing a type that can be manipulated with the typedvar_*
 * functions.
 **/
struct var_type_def_t {
  /**
   * The name of this type. Should not include spaces. Used for
   * debugging, log messages, and the controller API. */
  const char *name;
  /**
   * A function table for this type.
   */
  const struct var_type_fns_t *fns;
  /**
   * A pointer to a value that should be passed as the 'params' argument when
   * calling the functions in this type's function table.
   */
  const void *params;
  /**
   * A bitwise OR of one or more VTFLAG_* values, describing properties
   * for all values of this type.
   **/
  uint32_t flags;
};

#endif /* !defined(TOR_LIB_CONFMGT_VAR_TYPE_DEF_ST_H) */
