/*
# ----------------------------------------------------
# SPDX-FileCopyrightText: Ajeetha Kumari Venkatesan
#                         AsFigo Technologies, UK
# SPDX-License-Identifier: MIT
# ----------------------------------------------------
*/

#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#ifdef VERILATOR
#include "verilated_vpi.h" /* IEEE 1364 PLI VPI routine library  */
#else
#include "vpi_user.h" /* IEEE 1364 PLI VPI routine library  */
#endif
#include <string.h>

#define SUCCESS 0
#define ERR_FILE_OPEN 1
#define ERR_MOD_ITERATION 2
#define ERR_SYSTF_CALL 3
#define ERR_SYSTF_USAGE 4

/* Function Prototypes */
PLI_INT32 afPrHierCompiletf(PLI_BYTE8 *user_data);
PLI_INT32 afPrHierStartOfSim(struct t_cb_data *cb_data);
PLI_INT32 afPrHierCalltf(char *user_data);
void afPrHierCurNode(vpiHandle modHdl, FILE *afCsvOpFp);
void afPrHierTraverse(vpiHandle topModHdl, FILE *afCsvOpFp);
void afPrHierRegister();

/**********************************************************************
 * afPrHierRegister:
 * Registers the $afPrHier system task and start-of-simulation callback.
 * Sets up VPI and TF function pointers for simulation interaction.
 *********************************************************************/
void afPrHierRegister()
{
  s_vpi_systf_data tf_data;
  s_cb_data   cb_data_s;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = (PLI_BYTE8*)"$afPrHier";
  tf_data.calltf    = afPrHierCalltf;
  tf_data.compiletf = afPrHierCompiletf;
  vpi_register_systf(&tf_data);

  cb_data_s.reason    = cbStartOfSimulation;
  cb_data_s.cb_rtn    = afPrHierStartOfSim;
  cb_data_s.obj       = NULL;
  cb_data_s.time      = NULL;
  cb_data_s.value     = NULL;
  cb_data_s.user_data = NULL;
  vpi_register_cb(&cb_data_s); 
}

/* Array of startup routines, with afPrHierRegister as an entry point */
void (*vlog_startup_routines[] ) () =
{ 
  afPrHierRegister, 
  0 /* Last entry must be 0 */
};

/**********************************************************************
 * afPrHierCompiletf:
 * Verifies that the $afPrHier system task has been called without arguments.
 * Prints an error and halts simulation if incorrect usage is detected.
 *
 * Parameters:
 * - user_data: Pointer to any user-defined data, typically unused.
 *
 * Returns:
 * - SUCCESS on correct usage; otherwise, error codes on invalid usage.
 *********************************************************************/
PLI_INT32 afPrHierCompiletf(PLI_BYTE8 *user_data)
{
  vpiHandle systf_handle, arg_itr;

  (void)user_data;  // Mark user_data as unused
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  if (systf_handle == NULL) 
  {
    vpi_printf((PLI_BYTE8*)"ERROR: $afPrHier could not obtain a handle to the Task call\n");
    vpi_control(vpiFinish, 0);
    return ERR_SYSTF_CALL;
  }

  arg_itr = vpi_iterate(vpiArgument, systf_handle);
  if (arg_itr != NULL) 
  {
    vpi_printf((PLI_BYTE8*)"ERROR: $afPrHier does not require any arguments\n");
    vpi_control(vpiFinish, 0);
    return ERR_SYSTF_USAGE;
  }
  
  return SUCCESS;
}

/**********************************************************************
 * afPrHierCalltf:
 * Main function for the $afPrHier task. Opens a CSV file and retrieves
 * instance name for each module instance in the design,
 * writing it to the CSV file.
 *
 * Parameters:
 * - user_data: Pointer to any user-defined data, typically unused.
 *
 * Returns:
 * - SUCCESS on successful operation; otherwise, error codes on failure.
 *********************************************************************/
PLI_INT32 afPrHierCalltf(char *user_data)
{
  vpiHandle topModItr, topModHdl;
  const char *afOpCsvFname = "output_hier_info.csv";  
  FILE *afCsvOpFp = fopen(afOpCsvFname, "w");

  (void)user_data;  // Mark user_data as unused
  /* Open CSV file for writing */
  if (afCsvOpFp == NULL) {
    vpi_printf((PLI_BYTE8*)"ERROR: Failed to open CSV file: %s for writing\n", afOpCsvFname);
    return ERR_FILE_OPEN;
  }

  /* Write CSV header */
  fprintf(afCsvOpFp, "// AsFigo Verification utility - an app to print Instance hierarchy from VPI\n");
  fprintf(afCsvOpFp, "// Module_name, Instance_name\n");

  /* Get the iterator for top-level modules */
  topModItr = vpi_iterate(vpiModule, NULL); 
  if (topModItr == NULL) 
  {
    vpi_printf((PLI_BYTE8*)"ERROR: $afPrHier failed to obtain top module iterator\n");
    fclose(afCsvOpFp);
    return ERR_MOD_ITERATION;
  }

  /* Traverse and log each module's info */
  while ((topModHdl = vpi_scan(topModItr)) != NULL)
  {
    afPrHierTraverse(topModHdl, afCsvOpFp); 
  }

  vpi_printf((PLI_BYTE8*)"\n$afPrHier - a tree walker that prints each Module instance and its timescale information to a CSV file\n");
  vpi_printf((PLI_BYTE8*)"\nOutput file successfully created in: %s\n", afOpCsvFname);
  /* Close file and free iterator */
  fclose(afCsvOpFp);
  return SUCCESS;
}

/**********************************************************************
 * afPrHierTraverse:
 * Recursively traverses each module in the hierarchy, calling
 * afPrHierCurNode on each 
 *
 * Parameters:
 * - topModHdl: Handle to the current module being processed.
 * - afCsvOpFp: Pointer to the output CSV file.
 *********************************************************************/
void afPrHierTraverse(vpiHandle topModHdl, FILE *afCsvOpFp)
{
  vpiHandle childModItr, childModHdl;

  /* Process current module */
  afPrHierCurNode(topModHdl, afCsvOpFp);
  
  /* Iterate over child modules */
  childModItr = vpi_iterate(vpiModule, topModHdl);
  if (childModItr != NULL)
  {
    while ((childModHdl = vpi_scan(childModItr)) != NULL) 
    {
      afPrHierTraverse(childModHdl, afCsvOpFp);
    }
  }
}

/**********************************************************************
 * afPrHierCurNode:
 * Writes the module name, and instance name
 * to the CSV file.
 *
 * Parameters:
 * - modHdl: Handle to the module to be logged.
 * - afCsvOpFp: Pointer to the output CSV file.
 *********************************************************************/
void afPrHierCurNode(vpiHandle modHdl, FILE *afCsvOpFp)
{
  const char *modName  = strdup(vpi_get_str(vpiDefName, modHdl));
  const char *fullName = strdup(vpi_get_str(vpiFullName, modHdl));

  /* Write module data to CSV */
  fprintf(afCsvOpFp, "%s,%s\n",
      modName, fullName);
}


/**********************************************************************
 * afPrHierStartOfSim:
 * Callback function triggered at the start of the simulation.
 * Used for printing a header or initialization message.
 *
 * Parameters:
 * - cb_data: Pointer to callback data, unused in this context.
 *
 * Returns:
 * - SUCCESS always.
 *********************************************************************/
PLI_INT32 afPrHierStartOfSim(struct t_cb_data *cb_data)
{
  (void)cb_data;  // Mark cb_data as unused
  vpi_printf((PLI_BYTE8*)"\nAsFigo Verification Utility: $afPrHier VPI app. ");
  vpi_printf((PLI_BYTE8*)"It is a tree walker that prints each Module name and its instance information to a CSV file\n");
  return SUCCESS;
}
