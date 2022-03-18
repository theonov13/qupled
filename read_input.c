#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <argp.h>
#include "read_input.h"

// ----------------------------------------
// CONSTANTS AND DATA STRUCTURES
// ----------------------------------------

static bool debug_input;

// Documentation
static char doc[] =
  "The documentation is available at https://github.com/fedluc/STLS";

// Non-ascii characters to avoid using short options in the parser
#define ARGUMENT_THETA_SHORT 0x80
#define ARGUMENT_RS_SHORT 0x81
#define ARGUMENT_XMAX_SHORT 0x82
#define ARGUMENT_DX_SHORT 0x83
#define ARGUMENT_NL_SHORT 0x84
#define ARGUMENT_ITER_SHORT 0x85
#define ARGUMENT_MIN_ERR_SHORT 0x86
#define ARGUMENT_MIX_SHORT 0x87
#define ARGUMENT_MU_GUESS_SHORT 0x88
#define ARGUMENT_STLS_GUESS_SHORT 0x89
#define ARGUMENT_THEORY_SHORT 0x90
#define ARGUMENT_OMP_SHORT 0x91
#define ARGUMENT_QSTLS_GUESS_SHORT 0x92
#define ARGUMENT_QSTLS_FIXED_SHORT 0x93
#define ARGUMENT_QSTLS_IET_FIXED_SHORT 0x94
#define ARGUMENT_QSTLS_IET_STATIC_SHORT 0x95
#define ARGUMENT_DEBUG_SHORT 0x96
#define ARGUMENT_MODE_SHORT 0x97
#define ARGUMENT_GUESS_FILES_SHORT 0x98
#define ARGUMENT_IET_MAPPING_SHORT 0x99
#define ARGUMENT_VS_DRS_SHORT 0x100
#define ARGUMENT_VS_DT_SHORT 0x101
#define ARGUMENT_VS_ALPHA_SHORT 0x102
#define ARGUMENT_VS_THERMO_SHORT 0x103
#define ARGUMENT_VS_MIN_ERR_SHORT 0x104
#define ARGUMENT_VS_SOLVE_CSR_SHORT 0x105
#define ARGUMENT_VS_MIX_SHORT 0x106
#define ARGUMENT_DYN_DW_SHORT 0x107
#define ARGUMENT_DYN_WMAX_SHORT 0x108
#define ARGUMENT_DYN_XTARGET_SHORT 0x109

// Optional arguments
static struct argp_option options[] = {
				       
  {"Theta", ARGUMENT_THETA_SHORT, "1.0", 0,
   "Quantum degeneracy parameter"},
  
  {"rs", ARGUMENT_RS_SHORT, "1.0", 0,
   "Quantum coupling parameter"},
  
  {"xmax", ARGUMENT_XMAX_SHORT, "20.0", 0,
   "Cutoff for wave-vector grid"},
  
  {"dx", ARGUMENT_DX_SHORT, "0.1", 0,
   "Resolution for wave-vector grid"},

  {"nl", ARGUMENT_NL_SHORT, "128", 0,
   "Number of Matsubara frequencies"},

  {"iter", ARGUMENT_ITER_SHORT, "1000", 0,
   "Maximum number of iterations"},

  {"min-err", ARGUMENT_MIN_ERR_SHORT, "1e-5", 0,
   "Minimum error for convergence in the iterations"},

  {"mix", ARGUMENT_MIX_SHORT, "0.1", 0,
   "Mixing parameter for iterative solution"},

  {"mu-guess", ARGUMENT_MU_GUESS_SHORT, "-10,10", 0,
   "Initial guess for chemical potential"},

  {"stls-guess", ARGUMENT_STLS_GUESS_SHORT, "file", 0,
   "File used to load the stls and stls-iet schemes"},

  {"qstls-guess", ARGUMENT_QSTLS_GUESS_SHORT, "file", 0,
   "File used to load the qstls and qstls-iet schemes"},

  {"qstls-fix", ARGUMENT_QSTLS_FIXED_SHORT, "file", 0,
   "File used to load the fixed component of the density response function "
   "for the qslts scheme"},

  {"qstls-iet-fix", ARGUMENT_QSTLS_IET_FIXED_SHORT, "file", 0,
   "File used to load fixed component of the density response function "
   "for the qslts-iet scheme"},

  {"qstls-iet-static", ARGUMENT_QSTLS_IET_FIXED_SHORT, "0", 0,
   "Use static approximation to compute the auxiliary density response "
   "in the qstls-iet scheme (0 = off, 1 = on)"},

  {"theory", ARGUMENT_THEORY_SHORT, "STLS", 0,
   "Scheme to be solved"},

  {"omp", ARGUMENT_OMP_SHORT, "1",0,
   "Number of omp threads to use in the solution"},

  {"debug-input", ARGUMENT_DEBUG_SHORT, "0", 0,
   "Print content of the input structure on screen (0 = off, 1 = on)"},

  {"mode", ARGUMENT_MODE_SHORT, "static", 0,
   "Select working mode of the code (static, dynamic, guess)"},

  {"guess-files", ARGUMENT_GUESS_FILES_SHORT, "file1,file2", 0,
   "Name of the two text files used to write binary restart files"},

  {"iet-mapping", ARGUMENT_IET_MAPPING_SHORT, "standard", 0,
   "Mapping between quantum and classical state points for IET-based schemes"},

  {"vs-drs", ARGUMENT_VS_DRS_SHORT, "0.01", 0,
   "Resolution of the coupling parameter grid for the VS schemes"},

  {"vs-dt", ARGUMENT_VS_DT_SHORT, "0.01", 0,
   "Resolution of the degeneracy parameter grid for the VS schemes"},

  {"vs-alpha", ARGUMENT_VS_ALPHA_SHORT, "0.5", 0,
   "Initial guess for the free parameter in the VS schemes"},

  {"vs-thermo-file", ARGUMENT_VS_THERMO_SHORT, "file", 0,
   "File used to load the thermodynamic integration data for the VS schemes"},

  {"vs-min-err", ARGUMENT_VS_MIN_ERR_SHORT, "1e-3", 0,
   "Minimum error for convergence in the iterations for the VS schemes "},

  {"vs-mix", ARGUMENT_VS_MIX_SHORT, "1.0", 0,
   "Mixing parameter for iterative solution in the VS schemes "},
  
  {"vs-solve-csr", ARGUMENT_VS_SOLVE_CSR_SHORT, "1", 0,
   "Enforce CSR in the VS schemes (0 = off, 1 = on)"},

  {"dyn-dw", ARGUMENT_DYN_DW_SHORT, "0.1", 0,
   "Resolution for the frequency grid for the dynamic properties"},

  {"dyn-wmax", ARGUMENT_DYN_WMAX_SHORT, "20.0", 0,
   "Cutoff for the frequency grid for the dynamic properties"},

  {"dyn-xtarget", ARGUMENT_DYN_XTARGET_SHORT, "1.0", 0,
   "Wave-vector used to compute the dynamic properties"},
  
  { 0 }
  
};


// Command line parser
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{

  char *value;
  input *in = state->input;
  
  switch (key)
    {

    case ARGUMENT_DX_SHORT:
      in->dx = atof(arg);
      break;
      
    case  ARGUMENT_MIN_ERR_SHORT:
      in->err_min_iter = atof(arg);
      break;
      
    case  ARGUMENT_STLS_GUESS_SHORT:
      in->stls_guess_file = arg;
      break;
      
    case  ARGUMENT_QSTLS_GUESS_SHORT:
      in->qstls_guess_file = arg;
      break;
      
    case  ARGUMENT_QSTLS_FIXED_SHORT:
      in->qstls_fixed_file = arg;
      break;
      
    case  ARGUMENT_QSTLS_IET_FIXED_SHORT:
      in->qstls_iet_fixed_file = arg;
      break;

    case  ARGUMENT_QSTLS_IET_STATIC_SHORT:
      in->qstls_iet_static = atoi(arg);
      break;
      
    case  ARGUMENT_MU_GUESS_SHORT:
      value = strtok(arg, ",");
      if(value != NULL ) {
	in->mu_lo = atof(value);
      }
      else exit(EXIT_FAILURE);
      value = strtok(NULL, ",");
      if(value != NULL ) {
	in->mu_hi = atof(value);
      }
      else exit(EXIT_FAILURE);
      break;
      
    case ARGUMENT_ITER_SHORT:
      in->nIter = atoi(arg);
      break;
      
    case ARGUMENT_NL_SHORT:
      in->nl = atoi(arg);
      break;
      
    case ARGUMENT_MIX_SHORT:
      in->a_mix = atof(arg);
      break;
      
    case ARGUMENT_OMP_SHORT:
      in->nThreads = atoi(arg);
      break;
      
    case ARGUMENT_RS_SHORT:
      in->rs = atof(arg);
      break;
      
    case ARGUMENT_THETA_SHORT:
      in->Theta = atof(arg);
      break;
      
    case ARGUMENT_THEORY_SHORT:
      in->theory = arg;
      break;
      
    case  ARGUMENT_XMAX_SHORT:
      in->xmax = atof(arg);
      break;

    case  ARGUMENT_DEBUG_SHORT:
      debug_input = atoi(arg);
      break;

    case  ARGUMENT_MODE_SHORT:
      in->mode = arg;
      break;  

    case  ARGUMENT_GUESS_FILES_SHORT:
      value = strtok(arg, ",");
      if(value != NULL ) {
	in->guess_file1 = value;
      }
      else exit(EXIT_FAILURE);
      value = strtok(NULL, ",");
      if(value != NULL ) {
	in->guess_file2 = value;
      }
      else exit(EXIT_FAILURE);
      break;

    case ARGUMENT_IET_MAPPING_SHORT:
      in->iet_mapping = arg;
      break;

    case ARGUMENT_VS_DRS_SHORT:
      in->vs_drs = atof(arg);
      break;

    case ARGUMENT_VS_DT_SHORT:
      in->vs_dt = atof(arg);
      break;

    case ARGUMENT_VS_ALPHA_SHORT:
      in->vs_alpha = atof(arg);
      break;

    case ARGUMENT_VS_THERMO_SHORT:
      in->vs_thermo_file = arg;
      break;

    case  ARGUMENT_VS_MIN_ERR_SHORT:
      in->vs_err_min_iter = atof(arg);
      break;

    case  ARGUMENT_VS_MIX_SHORT:
      in->vs_a_mix = atof(arg);
      break;

    case  ARGUMENT_VS_SOLVE_CSR_SHORT:
      in->vs_solve_csr = atoi(arg);
      break;

    case  ARGUMENT_DYN_DW_SHORT:
      in->dyn_dW = atof(arg);
      break;

    case  ARGUMENT_DYN_WMAX_SHORT:
      in->dyn_Wmax = atof(arg);
      break;

    case  ARGUMENT_DYN_XTARGET_SHORT:
      in->dyn_xtarget = atof(arg);
      break;  
      
    case ARGP_KEY_ARG:
      if (state->arg_num > 0) 
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
      
    }
  
  return 0;
  
}

static struct argp argp = { options, parse_opt, 0, doc };

// ----------------------------------------
// FUNCTION TO READ INPUT DATA
// ----------------------------------------
void get_input(int argc, char **argv, input *in){

  // Default values for optional arguments
  set_default_parse_opt(in);

  // Parse command line
  argp_parse(&argp, argc, argv, 0, 0, in);

  // Get number of grid points
  get_grid_size(in);
  
  // Debug input
  if(debug_input) print_input(in);

  // Verify input validity
  check_input(in);
  
}


// -------------------------------------------------
// FUNCTION TO ASSIGN DEFAULT VALUES TO PARSER DATA
// -------------------------------------------------
void set_default_parse_opt(input *in){

  debug_input = false;
  in->stls_guess_file = NO_FILE_STR; // File with initial guess for STLS and STLS-IET schemes
  in->qstls_guess_file = NO_FILE_STR; // File with initial guess for QSTLS and QSTLS-IET schemes
  in->qstls_fixed_file = NO_FILE_STR; // File with fixed component of the density response for the QSTLS scheme
  in->qstls_iet_fixed_file = NO_FILE_STR; // File with fixed component of the density response for the QSTLS-IET scheme
  in->qstls_iet_static = 0; // Use static approximation to compute the auxiliary density response for the QSTLS-IET scheme
  in->Theta = 1.0; // Quantum degeneracy parameter
  in->rs = 1.0; // Quantum coupling parameter
  in->dx = 0.1; // Wave-vector grid resolution
  in->err_min_iter = 1e-5; // Minimum error for convergence in the iterative procedure
  in->a_mix = 0.1; // Mixing parameter for iterative procedure
  in->mu_lo = -10; // Initial guess for chemical potential (low bound)
  in->mu_hi = 10; // Initial guess for chemical potential (high bound)
  in->xmax = 20; // Cutoff for wave-vector grid
  in->nl = 128; // Number of Matsubara frequencies
  in->nIter = 1000; // Number of iterations
  in->theory = "STLS"; // Theory to solve
  in->nThreads = 1; // Number of OMP threads to use in the solution
  in->mode = "static"; // Working mode of the code
  in->guess_file1 = NO_FILE_STR; // File of the first file used to construct the guess (static structure factor)
  in->guess_file2 = NO_FILE_STR; // File of the second file used to construct the guess (static local field correction or auxiliary density response)
  in->iet_mapping = "standard"; // Mapping between the quantum and classical state points for the IET-based schemes
  in->vs_drs = 0.01; // Resolution of the coupling parameter grid for the VS schemes
  in->vs_dt = 0.01; // Resolution of the degeneracy parameter grid for the VS schemes
  in->vs_alpha = 0.5; // Initial guess for the free parameter in the VS schemes
  in->vs_thermo_file = NO_FILE_STR; // File with thermodynamic integration data for the VS schemes
  in->vs_err_min_iter = 1e-3; // Minimum error for convergence in the iterations for the VS schemes
  in->vs_a_mix = 1.0; // Mixing parameter for iterative procedure for the VS schemes 
  in->vs_solve_csr = 1; // Enforce CSR in the VS schemes
  in->dyn_dW = 0.1; // Resolution for the frequency grid for the dynamic properties
  in->dyn_Wmax = 20.0; // Cutoff for the frequency grid for the dynamic properties
  in->dyn_xtarget = 1.0; // Wave-vector used to compute the dynamic properties
  
}

// ------------------------------------------------
// FUNCTION TO COMPUTE THE NUMBER OF GRID POINTS
// ------------------------------------------------
void get_grid_size(input *in){
  in->nx = (int)floor(in->xmax/in->dx);
}


// ------------------------------------------------------------
// FUNCTION TO VERIFY THAT THE OPTIONS GIVEN IN INPUT ARE VALID
// ------------------------------------------------------------
void check_input(input *in){

  bool invalid_input = false;

  if (in->dx <= 0.0) {
    fprintf(stderr, "The resolution of the wave vector grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->xmax <= 0.0) {
    fprintf(stderr, "The cutoff of the wave vector grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->nIter < 0.0) {
    fprintf(stderr, "The number of iterations must be a positive number\n");
    invalid_input = true;
  }

  if (in->err_min_iter <= 0.0) {
    fprintf(stderr, "The minimum error for convergence must be larger than zero\n");
    invalid_input = true;
  }

  if (in->a_mix <= 0.0) {
    fprintf(stderr, "The mixing parameter must be larger than 0.0\n");
    invalid_input = true;
  }

  if (in->nl <= 0.0) {
    fprintf(stderr, "The number of Matsubara frequencies must be larger than zero\n");
    invalid_input = true;
  }

  if (in->nThreads <= 0.0) {
    fprintf(stderr, "The number of OMP threads must be larger than zero\n");
    invalid_input = true;
  }

  if (in->rs < 0.0) {
    fprintf(stderr, "The quantum coupling parameter must be larger than zero\n");
    invalid_input = true;
  }

  if (in->Theta < 0.0) {
    fprintf(stderr, "The quantum degeneracy parameter must be positive\n");
    invalid_input = true;
  }

  if (in->vs_drs <= 0.0) {
    fprintf(stderr, "The resolution of the coupling parameter grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->vs_dt <= 0.0) {
    fprintf(stderr, "The resolution of the degeneracy parameter grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->vs_alpha <= 0.0) {
    fprintf(stderr, "The free parameter for the VS schemes must be larger than zero\n");
    invalid_input = true;
  }

  if (in->vs_err_min_iter <= 0.0) {
    fprintf(stderr, "The minimum error for convergence must be larger than zero\n");
    invalid_input = true;
  }

  if (in->vs_a_mix <= 0.0) {
    fprintf(stderr, "The mixing parameter must be larger than 0.0\n");
    invalid_input = true;
  }

  if (in->dyn_dW <= 0.0) {
    fprintf(stderr, "The resolution of the frequency grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->dyn_Wmax <= 0.0) {
    fprintf(stderr, "The cutoff of the frequency grid must be larger than zero\n");
    invalid_input = true;
  }

  if (in->dyn_xtarget <= 0.0) {
    fprintf(stderr, "The wave-vector used to compute the dynamic properties  must be larger than zero\n");
    invalid_input = true;
  }

  
  if (invalid_input) exit(EXIT_FAILURE);
  
}

// ------------------------------------------------
// FUNCTION TO DEBUG THE INPUT
// ------------------------------------------------
void print_input(input *in){
  
  printf("------ Input parameters -------------\n");
  printf("File for initial guess (STLS): %s\n", in->stls_guess_file);
  printf("File for initial guess (qSTLS): %s\n", in->qstls_guess_file);
  printf("File for fixed component (qSTLS): %s\n", in->qstls_fixed_file);
  printf("File for fixed component (qSTLS-IET): %s\n", in->qstls_iet_fixed_file);
  printf("Static approximation (qSTLS-IET): %d\n", in->qstls_iet_static);
  printf("Theory: %s\n", in->theory);
  printf("Quantum degeneracy parameter: %f\n", in->Theta);
  printf("Quantum coupling parameter: %f\n", in->rs);
  printf("Wave-vector resolution: %f\n", in->dx);
  printf("Error for convergence: %.5e\n", in->err_min_iter);
  printf("Mixing parameter: %f\n", in->a_mix);
  printf("Chemical potential (low and high bound): %f %f\n", 
	 in->mu_lo, in->mu_hi);
  printf("Wave-vector cutoff: %f\n", in->xmax);
  printf("Number of Matsubara frequencies: %d\n", in->nl);
  printf("Number of grid points: %d\n", in->nIter);  
  printf("Maximum number of iterations: %d\n", in->nIter);
  printf("Number of threads: %d\n", in->nThreads);
  printf("Mode: %s\n", in->mode);
  printf("Guess file 1: %s\n", in->guess_file1);
  printf("Guess file 2: %s\n", in->guess_file2);
  printf("IET mapping: %s\n", in->iet_mapping);
  printf("Coupling parameter resolution (VS schemes): %f\n", in->vs_drs);
  printf("Degeneracy parameter resolution (VS schemes): %f\n", in->vs_dt);
  printf("Free parameter for VS schemes: %f\n", in->vs_alpha);
  printf("File for thermodynamic integration (VS): %s\n", in->vs_thermo_file);
  printf("Error for convergence (VS): %f\n", in->vs_err_min_iter);
  printf("Mixing parameter (VS): %f\n", in->vs_a_mix);
  printf("Enforce CSR (VS): %d\n", in->vs_solve_csr);
  printf("Frequency resolution (dynamic): %f\n", in->dyn_dW);
  printf("Frequency cutoff (dynamic): %f\n", in->dyn_Wmax);
  printf("Target wave-vector (dynamic): %f\n", in->dyn_xtarget);
  printf("-------------------------------------\n");
  
}
