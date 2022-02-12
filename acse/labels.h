/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * labels.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Internal functions for label management.
 */

#ifndef LABELS_H
#define LABELS_H

typedef struct t_axe_label {
   unsigned int labelID; /* Unique identifier for the label */
   char *name;           /* Name of the label. If NULL, the name will be 
                          * automatically generated in the form L<ID>. */
   int global;           /* zero for local labels, non-zero for global labels.*/
   int isAlias;
} t_axe_label;

struct t_axe_label_manager;

/* Typedef for the struct t_axe_label_manager */
typedef struct t_axe_label_manager t_axe_label_manager;


/* initialize the memory structures for the label manager */
extern t_axe_label_manager * initializeLabelManager();

/* finalize an instance of `t_axe_label_manager' */
extern void finalizeLabelManager(t_axe_label_manager *lmanager);

/* Reserve a new label identifier and return the identifier to the caller. */
extern t_axe_label * newLabelID(t_axe_label_manager *lmanager, int global);

/* assign the given label identifier to the next instruction. Returns
 * FALSE if an error occurred; otherwise true */
extern t_axe_label * assignLabelID(t_axe_label_manager *lmanager, t_axe_label *label);

/* retrieve the label that will be assigned to the next instruction */
extern t_axe_label * getLastPendingLabel(t_axe_label_manager *lmanager);

/* get the number of labels inside the list of labels */
extern int getLabelCount(t_axe_label_manager *lmanager);

/* Enumerate the labels in the given label manager. `state' must be a pointer
 * to a void* variable initially set to NULL. At each invocation, the function
 * returns the next label in the list. When the list of labels is exhausted,
 * the function returns NULL. */
extern t_axe_label *enumLabels(t_axe_label_manager *lmanager, void **state);

/* return TRUE if the two labels hold the same identifier */
extern int compareLabels(t_axe_label *labelA, t_axe_label *labelB);

/* test if a label will be assigned to the next instruction */
extern int isAssigningLabel(t_axe_label_manager *lmanager);

/* Sets the name of a label to the specified string. Note: if another label
 * with the same name already exists, the name assigned to this label will be
 * modified to remove any ambiguity. */
extern void setLabelName(t_axe_label_manager *lmanager, t_axe_label *label,
      const char *name);

/* Returns a dynamically allocated string that contains the name of the given
 * label. */
extern char *getLabelName(t_axe_label *label);

#endif
