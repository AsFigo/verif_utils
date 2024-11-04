/*
# ----------------------------------------------------
# SPDX-FileCopyrightText: Ajeetha Kumari Venkatesan
#                         AsFigo Technologies, UK
# SPDX-License-Identifier: MIT
# ----------------------------------------------------
*/

#include <stdlib.h>    /* ANSI C standard library */
#include <stdio.h>     /* ANSI C standard input/output library */
#include "vpi_user.h" /* IEEE 1364 PLI VPI routine library  */
#include "veriuser.h" /* IEEE 1364 PLI TF routine library  */

#define SUCCESS 0
#define ERR_FILE_OPEN 1
#define ERR_MOD_ITERATION 2
#define ERR_SYSTF_CALL 3
#define ERR_SYSTF_USAGE 4

/* Function Prototypes */
PLI_INT32 afPrTsCompiletf(PLI_BYTE8 *user_data);
PLI_INT32 afPrTsStartOfSim(struct t_cb_data *cb_data);
const char *tu2Str(int curTU);
PLI_INT32 afPrTsCalltf(char *user_data);
void afPrTsCurNode(vpiHandle modHdl, FILE *afCsvOpFp);
void afPrTsTraverse(vpiHandle topModHdl, FILE *afCsvOpFp);
void afPrTsregister();

/**********************************************************************
 * afPrTsregister:
 * Registers the $afPrTs system task and start-of-simulation callback.
 * Sets up VPI and TF function pointers for simulation interaction.
 *********************************************************************/
void afPrTsregister()
{
  s_vpi_systf_data tf_data;
  s_cb_data   cb_data_s;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$afPrTs";
  tf_data.calltf    = afPrTsCalltf;
  tf_data.compiletf = afPrTsCompiletf;
  vpi_register_systf(&tf_data);

  cb_data_s.reason    = cbStartOfSimulation;
  cb_data_s.cb_rtn    = afPrTsStartOfSim;
  cb_data_s.obj       = NULL;
  cb_data_s.time      = NULL;
  cb_data_s.value     = NULL;
  cb_data_s.user_data = NULL;
  vpi_register_cb(&cb_data_s); 
}

/* Array of startup routines, with afPrTsregister as an entry point */
void (*vlog_startup_routines[] ) () =
{ 
  afPrTsregister, 
  0 /* Last entry must be 0 */
};

/**********************************************************************
 * afPrTsCompiletf:
 * Verifies that the $afPrTs system task has been called without arguments.
 * Prints an error and halts simulation if incorrect usage is detected.
 *
 * Parameters:
 * - user_data: Pointer to any user-defined data, typically unused.
 *
 * Returns:
 * - SUCCESS on correct usage; otherwise, error codes on invalid usage.
 *********************************************************************/
PLI_INT32 afPrTsCompiletf(PLI_BYTE8 *user_data)
{
  vpiHandle systf_handle, arg_itr;

  (void)user_data;  // Mark user_data as unused
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  if (systf_handle == NULL) 
  {
    vpi_printf("ERROR: $afPrTs could not obtain a handle to the Task call\n");
    vpi_control(vpiFinish, 0);
    return ERR_SYSTF_CALL;
  }

  arg_itr = vpi_iterate(vpiArgument, systf_handle);
  if (arg_itr != NULL) 
  {
    vpi_printf("ERROR: $afPrTs does not require any arguments\n");
    vpi_control(vpiFinish, 0);
    return ERR_SYSTF_USAGE;
  }
  
  return SUCCESS;
}

/**********************************************************************
 * afPrTsCalltf:
 * Main function for the $afPrTs task. Opens a CSV file and retrieves
 * time unit and precision information for each module in the design,
 * writing it to the CSV file.
 *
 * Parameters:
 * - user_data: Pointer to any user-defined data, typically unused.
 *
 * Returns:
 * - SUCCESS on successful operation; otherwise, error codes on failure.
 *********************************************************************/
PLI_INT32 afPrTsCalltf(char *user_data)
{
  vpiHandle topModItr, topModHdl;
  const char *afOpCsvFname = "output_tscale_info.csv";  
  FILE *afCsvOpFp = fopen(afOpCsvFname, "w");

  (void)user_data;  // Mark user_data as unused
  /* Open CSV file for writing */
  if (afCsvOpFp == NULL) {
    vpi_printf("ERROR: Failed to open CSV file: %s for writing\n", afOpCsvFname);
    return ERR_FILE_OPEN;
  }

  /* Write CSV header */
  fprintf(afCsvOpFp, "Module, Time unit, Time Precision\n");

  /* Get the iterator for top-level modules */
  topModItr = vpi_iterate(vpiModule, NULL); 
  if (topModItr == NULL) 
  {
    vpi_printf("ERROR: $afPrTs failed to obtain top module iterator\n");
    fclose(afCsvOpFp);
    return ERR_MOD_ITERATION;
  }

  /* Traverse and log each module's time unit and precision */
  while ((topModHdl = vpi_scan(topModItr)) != NULL)
  {
    afPrTsTraverse(topModHdl, afCsvOpFp); 
  }

  vpi_printf("\n$afPrTs - a tree walker that prints each Module name and its timescale information to a CSV file\n");
  vpi_printf("\nOutput file successfully created in: %s\n", afOpCsvFname);
  /* Close file and free iterator */
  fclose(afCsvOpFp);
  return SUCCESS;
}

/**********************************************************************
 * afPrTsTraverse:
 * Recursively traverses each module in the hierarchy, calling
 * afPrTsCurNode on each to write time unit and precision data to CSV.
 *
 * Parameters:
 * - topModHdl: Handle to the current module being processed.
 * - afCsvOpFp: Pointer to the output CSV file.
 *********************************************************************/
void afPrTsTraverse(vpiHandle topModHdl, FILE *afCsvOpFp)
{
  vpiHandle childModItr, childModHdl;

  /* Process current module */
  afPrTsCurNode(topModHdl, afCsvOpFp);
  
  /* Iterate over child modules */
  childModItr = vpi_iterate(vpiModule, topModHdl);
  if (childModItr != NULL)
  {
    while ((childModHdl = vpi_scan(childModItr)) != NULL) 
    {
      afPrTsTraverse(childModHdl, afCsvOpFp);
    }
  }
}

/**********************************************************************
 * afPrTsCurNode:
 * Writes the module name, time unit, and precision of the given module
 * to the CSV file.
 *
 * Parameters:
 * - modHdl: Handle to the module to be logged.
 * - afCsvOpFp: Pointer to the output CSV file.
 *********************************************************************/
void afPrTsCurNode(vpiHandle modHdl, FILE *afCsvOpFp)
{
  int modTUnit, modTPrec;

  /* Retrieve time unit and precision for the module */
  modTUnit = vpi_get(vpiTimeUnit, modHdl);
  modTPrec = vpi_get(vpiTimePrecision, modHdl);

  /* Write module data to CSV */
  fprintf(afCsvOpFp, "%s, %s, %s\n",
      vpi_get_str(vpiDefName, modHdl),
      tu2Str(modTUnit), tu2Str(modTPrec));
}

/**********************************************************************
 * tu2Str:
 * Converts an integer time unit code to a readable string.
 *
 * Parameters:
 * - curTU: Integer code representing the time unit.
 *
 * Returns:
 * - A string representing the human-readable time unit.
 *********************************************************************/
const char *tu2Str(int curTU)
{
  switch (curTU) 
  {
    case -15: return " fs";
    case -14: return " 10 fs";
    case -13: return " 100 fs";
    case -12: return " ps";
    case -11: return " 10 ps";
    case -10: return " 100 ps";
    case  -9: return " ns";
    case  -8: return " 10 ns";
    case  -7: return " 100 ns";
    case  -6: return " us";
    case  -5: return " 10 us";
    case  -4: return " 100 us";
    case  -3: return " ms";
    case  -2: return " 10 ms";
    case  -1: return " 100 ms";
    case   0: return " s";
    case   1: return " 10 s";
    case   2: return " 100 s";
    default:  return "Unknown Time Unit";
  }
}

/**********************************************************************
 * afPrTsStartOfSim:
 * Callback function triggered at the start of the simulation.
 * Used for printing a header or initialization message.
 *
 * Parameters:
 * - cb_data: Pointer to callback data, unused in this context.
 *
 * Returns:
 * - SUCCESS always.
 *********************************************************************/
PLI_INT32 afPrTsStartOfSim(struct t_cb_data *cb_data)
{
  (void)cb_data;  // Mark cb_data as unused
  vpi_printf("\nAsFigo Verification Utility: $afPrTs VPI app. ");
  vpi_printf("It is a tree walker that prints each Module name and its timescale information to a CSV file\n");
  return SUCCESS;
}
