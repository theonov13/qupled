#include "util.hpp"
#include "input.hpp"

using namespace std;
using namespace parallelUtil;

// -----------------------------------------------------------------
// Input class
// -----------------------------------------------------------------

void Input::setCoupling(const double &rs_){
  if (rs_ < 0) {
    MPI::throwError("The quantum coupling parameter can't be negative");
  }
  this->rs = rs_;
}

void Input::setDegeneracy(const double &Theta_){
  if (Theta_ < 0.0) {
    MPI::throwError("The quantum degeneracy parameter can't be negative");
  }
  this->Theta = Theta_;
}

void Input::setTheory(const string &theory_){
  const vector<string> cTheories = {"RPA", "ESA", "STLS",
				    "STLS-HNC", "STLS-IOI",
				    "STLS-LCT", "VSSTLS"};
  const vector<string> qTheories = {"QSTLS", "QSTLS-HNC",
				    "QSTLS-IOI", "QSTLS-LCT", "QVSSTLS"};
  isClassicTheory = count(cTheories.begin(), cTheories.end(), theory_) != 0;
  isQuantumTheory = count(qTheories.begin(), qTheories.end(), theory_) != 0;
  if (!isClassicTheory && !isQuantumTheory) {
    MPI::throwError("Invalid dielectric theory: " + theory_);
  }
  // A theory can't both be classical and quantum at the same time
  assert(!isClassicTheory || !isQuantumTheory);
  this->theory = theory_;
}

void Input::setInt2DScheme(const string &int2DScheme){
  const vector<string> schemes = {"full", "segregated"};
  if (count(schemes.begin(), schemes.end(), int2DScheme) == 0) {
    MPI::throwError("Unknown scheme for 2D integrals: " + int2DScheme);
  }
  this->int2DScheme = int2DScheme;
}

void Input::setIntError(const double &intError){
  if (intError <= 0) {
    MPI::throwError("The accuracy for the integral computations must be larger than zero");
  }
  this->intError = intError;
}


void Input::setNThreads(const int &nThreads){
  if (nThreads <= 0) {
    MPI::throwError("The number of threads must be larger than zero");
  }
  this->nThreads = nThreads;
}

void Input::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  cout << "Coupling parameter = " << rs << endl;
  cout << "Degeneracy parameter = " << Theta << endl;
  cout << "Number of OMP threads = " << nThreads << endl;
  cout << "Scheme for 2D integrals = " << int2DScheme << endl;
  cout << "Integral relative error = " << intError << endl;
  cout << "Theory to be solved = " << theory << endl;
}

bool Input::isEqual(const Input &in) const {
  return ( int2DScheme == in.int2DScheme &&
	   nThreads == in.nThreads &&
	   rs == in.rs &&
	   theory == in.theory &&
	   Theta == in.Theta ); 
}

// -----------------------------------------------------------------
// RpaInput class
// -----------------------------------------------------------------

void RpaInput::setChemicalPotentialGuess(const vector<double> &muGuess){
  if (muGuess.size() != 2 || muGuess[0] >= muGuess[1]) {
    MPI::throwError("Invalid guess for chemical potential calculation");
  }
  this->muGuess = muGuess;
}

void RpaInput::setNMatsubara(const int &nl){
  if (nl < 0) {
    MPI::throwError("The number of matsubara frequencies can't be negative");
  }
  this->nl = nl;
}

void RpaInput::setWaveVectorGridRes(const double &dx){
  if (dx <= 0.0) {
    MPI::throwError("The wave-vector grid resolution must be larger than zero");
  }
  this->dx = dx;
}

void RpaInput::setWaveVectorGridCutoff(const double &xmax){
  if (xmax <= 0.0) {
    MPI::throwError("The wave-vector grid cutoff must be larger than zero");
  }
  this->xmax = xmax;
}

void RpaInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  Input::print();
  cout << "Guess for chemical potential = " << muGuess.at(0) << "," << muGuess.at(1) << endl;
  cout << "Number of Matsubara frequencies = " << nl << endl;
  cout << "Wave-vector resolution = " << dx << endl;
  cout << "Wave-vector cutoff = " << xmax << endl;
}

bool RpaInput::isEqual(const RpaInput &in) const {
  return ( Input::isEqual(in) &&
	   dx == in.dx && 
	   muGuess == in.muGuess &&
	   nl == in.nl &&
	   xmax == in.xmax );
}

// -----------------------------------------------------------------
// STLSInput class
// -----------------------------------------------------------------

void StlsInput::setErrMin(const double &errMin){
  if (errMin <= 0.0) {
    MPI::throwError("The minimum error for convergence must be larger than zero");
  }
  this->errMin = errMin;
}

void StlsInput::setMixingParameter(const double &aMix){
  if (aMix < 0.0 || aMix > 1.0) {
    MPI::throwError("The mixing parameter must be a number between zero and one");
  }
  this->aMix = aMix;
}

void StlsInput::setNIter(const int &nIter){
  if (nIter < 0) {
    MPI::throwError("The maximum number of iterations can't be negative");
  }
  this->nIter = nIter; 
}

void StlsInput::setOutIter(const int &outIter){
  if (outIter < 0) {
    MPI::throwError("The output frequency can't be negative");
  }
  this->outIter = outIter; 
}

void StlsInput::setIETMapping(const string &IETMapping){
  const vector<string> mappings = {"standard", "sqrt", "linear"};
  if (count(mappings.begin(), mappings.end(), IETMapping) == 0) {
    MPI::throwError("Unknown IET mapping: " + IETMapping);
  }
  this->IETMapping = IETMapping;
}

void StlsInput::setRecoveryFileName(const string &recoveryFileName){
  this->recoveryFileName = recoveryFileName;
}

void StlsInput::setGuess(const SlfcGuess &guess){
  if (guess.wvg.size() < 3 || guess.slfc.size() < 3) {
    MPI::throwError("The initial guess does not contain enough points");
  }
  if (guess.wvg.size() != guess.slfc.size()) {
    MPI::throwError("The initial guess is inconsistent");
  }
  this->guess = guess;
}

void StlsInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  RpaInput::print();
  cout << "Iet mapping scheme = " << IETMapping << endl;
  cout << "Maximum number of iterations = " << nIter << endl;
  cout << "Minimum error for convergence = " << errMin << endl;
  cout << "Mixing parameter = " << aMix << endl;
  cout << "Output frequency = " << outIter << endl;
  cout << "File with recovery data = " << recoveryFileName << endl;
}

bool StlsInput::isEqual(const StlsInput &in) const {
  return ( Input::isEqual(in) &&
	   aMix == in.aMix && 
	   errMin == in.errMin &&
	   IETMapping == in.IETMapping &&
	   nIter == in.nIter &&
	   outIter == in.outIter &&
	   recoveryFileName == in.recoveryFileName &&
	   guess == in.guess);
}

// -----------------------------------------------------------------
// QStlsInput class
// -----------------------------------------------------------------

void QstlsInput::setFixed(const string &fixed){
  this->fixed = fixed;
} 

void QstlsInput::setFixedIet(const string &fixedIet){
  this->fixedIet = fixedIet;
} 

void QstlsInput::setGuess(const QstlsGuess &guess){
  if (guess.wvg.size() < 3 || guess.ssf.size() < 3) {
    MPI::throwError("The initial guess does not contain enough points");
  }
  bool consistentGuess = guess.wvg.size() == guess.ssf.size();
  const size_t nl = guess.matsubara;
  if (guess.adr.size(0) > 0) {
    consistentGuess = consistentGuess
      && guess.adr.size(0) == guess.wvg.size()
      && guess.adr.size(1) == nl;
  }
  if (!consistentGuess) {
    MPI::throwError("The initial guess is inconsistent");
  }
  this->guess = guess;
}

void QstlsInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  StlsInput::print();
  cout << "File with fixed adr component = " << fixed  << endl;
  cout << "File with fixed adr component (iet) = " << fixedIet  << endl;
}

bool QstlsInput::isEqual(const QstlsInput &in) const {
  return (StlsInput::isEqual(in) &&
	  fixed == in.fixed &&
	  fixedIet == in.fixedIet &&
	  guess == in.guess );
}

// -----------------------------------------------------------------
// VSInput class
// -----------------------------------------------------------------

void VSInput::setCouplingResolution(const double &drs) {
  if (drs <= 0) {
    MPI::throwError("The coupling parameter resolution must be larger than zero");
  }
  this->drs = drs;
}

void VSInput::setDegeneracyResolution(const double &dTheta) {
  if (dTheta <= 0) {
    MPI::throwError("The degeneracy parameter resolution must be larger than zero");
  }
  this->dTheta = dTheta;
}

void VSInput::setAlphaGuess(const vector<double>  &alphaGuess) {
  if (alphaGuess.size() != 2 || alphaGuess[0] >= alphaGuess[1]) {
    MPI::throwError("Invalid guess for free parameter calculation");
  }
  this->alphaGuess = alphaGuess;
}


void VSInput::setErrMinAlpha(const double &errMinAlpha){
   if (errMinAlpha <= 0.0) {
    MPI::throwError("The minimum error for convergence must be larger than zero");
  }
  this->errMinAlpha = errMinAlpha;
}

void VSInput::setNIterAlpha(const int &nIterAlpha){
  if (nIterAlpha < 0) {
    MPI::throwError("The maximum number of iterations can't be negative");
  }
  this->nIterAlpha = nIterAlpha; 
}

void VSInput::setFreeEnergyIntegrand(const FreeEnergyIntegrand& fxcIntegrand) {
  if (fxcIntegrand.integrand.size() < 3) {
    MPI::throwError("The free energy integrand does not contain enough temperature points");
  }
  for (const auto& fxci : fxcIntegrand.integrand) {
    if (fxci.size() != fxcIntegrand.integrand[0].size()) {
      MPI::throwError("The free energy integrand is inconsistent");
    }
  }
  if (fxcIntegrand.grid.size() < 3 || fxcIntegrand.integrand[0].size() < 3) {
    MPI::throwError("The free energy integrand does not contain enough points");
  }
  if (fxcIntegrand.grid.size() != fxcIntegrand.integrand[0].size()) {
    MPI::throwError("The free energy integrand is inconsistent");
  }
  this->fxcIntegrand = fxcIntegrand;
}

void VSInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  cout << "Guess for the free parameter = " << alphaGuess.at(0) << "," << alphaGuess.at(1) << endl;
  cout << "Resolution for the coupling parameter grid = " << drs << endl;
  cout << "Resolution for the degeneracy parameter grid = " << dTheta << endl;
  cout << "Minimum error for convergence (alpha) = " << errMinAlpha << endl;
  cout << "Maximum number of iterations (alpha) = " << nIterAlpha << endl;
}

bool VSInput::isEqual(const VSInput &in) const {
  return ( alphaGuess == in.alphaGuess &&
	   drs == in.drs &&
	   dTheta == in.dTheta &&
	   errMinAlpha == in.errMinAlpha &&
	   nIterAlpha == in.nIterAlpha &&
	   fxcIntegrand == in.fxcIntegrand);
}

// -----------------------------------------------------------------
// VSStlsInput class
// -----------------------------------------------------------------

void VSStlsInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  StlsInput::print();
  VSInput::print();
}

bool VSStlsInput::isEqual(const VSStlsInput &in) const {
  return StlsInput::isEqual(in) && VSInput::isEqual(in);
}

// -----------------------------------------------------------------
// QVSStlsInput class
// -----------------------------------------------------------------

void QVSStlsInput::print() const {
  if (!MPI::isRoot()) {
    return;
  }
  QstlsInput::print();
  VSInput::print();
}

bool QVSStlsInput::isEqual(const QVSStlsInput &in) const {
  return QstlsInput::isEqual(in) && VSInput::isEqual(in);
}
