#!/usr/bin/env python

import sys
import os
from shutil import rmtree
from glob import glob
import zipfile as zf
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import qupled.qupled as qp

class Stls():
            
    # Constructor
    def __init__(self,
                 coupling,
                 degeneracy,
                 chemicalPotential = None,
                 cutoff = None,
                 error = None,
                 mixing = None,
                 guess = None,
                 iterations = None,
                 matsubara = None,
                 outputFrequency = None,
                 recoveryFile = None,
                 resolution = None ):
        # Allowed theories
        self.allowedTheories = ["STLS"]
        # Default inputs
        self.inputs = qp.StlsInput(coupling, degeneracy, "STLS")
        # Scheme to solve and associated input and solution
        self.scheme = None
        self.schemeInputs = None
        self.schemeSolution = None
        # File to store output on disk
        self.hdfFileName = None
        # Non-default inputs
        if (chemicalPotential is not None):
            self.inputs.chemicalPotential = chemicalPotential
        if (cutoff is not None):
            self.inputs.cutoff = cutoff
        if (error is not None):
            self.inputs.error = error
        if (mixing is not None):
            self.inputs.mixing = mixing
        if (guess is not None):
            self.inputs.guess = guess
        if (iterations is not None):
            self.inputs.iterations = iterations
        if (matsubara is not None):
            self.inputs.matsubara = matsubara
        if (outputFrequency is not None):
            self.inputs.outputFrequency = outputFrequency
        if (recoveryFile is not None):
            self.inputs.recoveryFile = recoveryFile
        if (resolution is not None):
            self.inputs.resolution = resolution
        
    # Check input before computing
    def checkInputs(self):
        if (self.inputs.theory not in self.allowedTheories):
            sys.exit("Invalid dielectric theory")

    # Compute
    def compute(self):
        self.checkInputs()
        self.schemeInputs = self.inputs
        self.scheme = qp.Stls(self.schemeInputs)
        status = self.scheme.compute()
        self.checkStatusAndClean(status)        
        self.setHdfFile()
        self.save()

    # Check that the dielectric scheme was solved without errors
    def checkStatusAndClean(self, status):
        if (status == 0):
            if os.path.isfile(self.scheme.recovery) : os.remove(self.scheme.recovery)
            print("Dielectric theory solved successfully!")
        else:
            sys.exit("Error while solving the dielectric theory")
    
    # Save results to disk
    def setHdfFile(self):
        self.hdfFileName = "rs%5.3f_theta%5.3f_%s.h5" % (self.schemeInputs.coupling,
                                                         self.schemeInputs.degeneracy,
                                                         self.schemeInputs.theory)
    
    def save(self):
        pd.DataFrame({
            "coupling" : self.schemeInputs.coupling,
            "degeneracy" : self.schemeInputs.degeneracy,
            "theory" : self.schemeInputs.theory,
            "error" : self.schemeInputs.error,
            "resolution" : self.schemeInputs.resolution,
            "cutoff" : self.schemeInputs.cutoff,
            "matsubara" : self.schemeInputs.matsubara
            }, index=["inputs"]).to_hdf(self.hdfFileName, key="inputs", mode="w")
        pd.DataFrame(self.scheme.idr).to_hdf(self.hdfFileName, key="idr")
        pd.DataFrame(self.scheme.sdr).to_hdf(self.hdfFileName, key="sdr")
        pd.DataFrame(self.scheme.slfc).to_hdf(self.hdfFileName, key="slfc")
        pd.DataFrame(self.scheme.ssf).to_hdf(self.hdfFileName, key="ssf")
        pd.DataFrame(self.scheme.ssfHF).to_hdf(self.hdfFileName, key="ssfHF")
        pd.DataFrame(self.scheme.wvg).to_hdf(self.hdfFileName, key="wvg")
        
    # Compute radial distribution function
    def computeRdf(self, rdfGrid, writeToHdf = True):
        if (self.schemeInputs == None):
            sys.exit("No solution to compute the radial distribution function")
        rdf = self.scheme.getRdf(rdfGrid)
        if (writeToHdf) :
            pd.DataFrame(rdfGrid).to_hdf(self.hdfFileName, key="rdfGrid", mode="r+")
            pd.DataFrame(rdf).to_hdf(self.hdfFileName, key="rdf", mode="r+")
        return rdf
        
    # Plot results        
    def plot(self, toPlot, matsubara = None, rdfGrid = np.arange(0.01, 10.0, 0.01)):
        wvg = self.scheme.wvg
        xlabel = "Wave vector"
        if ("idr" in toPlot):
            self.plotIdr(matsubara)
        if ("rdf" in toPlot):
            self.plotRdf(rdfGrid)
        if ("sdr" in toPlot):
            Plot.plot1D(wvg, self.scheme.sdr, xlabel, "Static density response")
        if ("slfc" in toPlot):
            Plot.plot1D(wvg, self.scheme.slfc, xlabel, "Static local field correction")
        if ("ssf" in toPlot):
            Plot.plot1D(wvg, self.scheme.ssf, xlabel, "Static structure factor")
        if ("ssfHF" in toPlot):
            Plot.plot1D(wvg, self.scheme.ssfHF, xlabel, "Hartree-Fock static structure factor")
        
    def plotIdr(self, matsubara):
        if (matsubara is None) : matsubara = np.arange(self.inputs.matsubara)
        Plot.plot1DParametric(self.scheme.wvg, self.scheme.idr,
                              "Wave vector", "Ideal density response",
                              matsubara)
        
    def plotRdf(self, rdfGrid):
        rdf = self.computeRdf(rdfGrid)
        Plot.plot1D(rdfGrid, rdf, "Inter-particle distance", "radial distribution function")
        
        
class StlsIet(Stls):
            
    # Constructor
    def __init__(self,
                 coupling,
                 degeneracy,
                 theory,
                 chemicalPotential = None,
                 cutoff = None,
                 error = None,
                 mapping = None,
                 mixing = None,
                 guess = None,
                 iterations = None,
                 matsubara = None,
                 outputFrequency = None,
                 recoveryFile = None,
                 scheme2DIntegrals = None,
                 resolution = None ):
        # Call parent constructor
        super().__init__(coupling, degeneracy,
                         chemicalPotential, cutoff, error,
                         mixing, guess, iterations,
                         matsubara, outputFrequency,
                         recoveryFile, resolution)
        # Allowed theories
        self.allowedTheories = ["STLS-HNC", "STLS-IOI", "STLS-LCT"]
        # Set theory
        self.inputs.theory = theory
        self.checkInputs()
        # File to store output on disk
        self.hdfFileName = "rs%5.3f_theta%5.3f_%s.h5" % (self.inputs.coupling,
                                                           self.inputs.degeneracy,
                                                           self.inputs.theory)
        # Non-default inputs
        if (mapping is not None):
            self.inputs.iet = mapping
        if (scheme2DIntegrals is not None):
            self.inputs.int2DScheme = scheme2DIntegrals
            
    # Plot results        
    def plot(self, toPlot, matsubara = None, rdfGrid = np.arange(0.01, 10.0, 0.01)):
        super().plot(toPlot, matsubara, rdfGrid)
        if ("bf" in toPlot):
            Plot.plot1D(self.scheme.wvg, self.scheme.bf, "Wave vector", "Bridge function adder")
        
    # Save results to disk
    def save(self):
        super().save()
        pd.DataFrame(self.scheme.bf).to_hdf(self.hdfFileName, key="bf")



class Qstls(Stls):
            
    # Constructor
    def __init__(self,
                 coupling,
                 degeneracy,
                 chemicalPotential = None,
                 cutoff = None,
                 error = None,
                 fixed = None,
                 mixing = None,
                 guess = None,
                 iterations = None,
                 matsubara = None,
                 outputFrequency = None,
                 recoveryFile = None,
                 resolution = None,
                 threads = None):
        # Call parent constructor
        super().__init__(coupling, degeneracy,
                         chemicalPotential, cutoff, error,
                         mixing, None, iterations,
                         matsubara, outputFrequency,
                         recoveryFile, resolution)
        # Allowed theories
        self.allowedTheories = ["QSTLS"]
        # Set theory
        self.inputs.theory = "QSTLS"
        # Qstls inputs
        self.qInputs = qp.QstlsInput()
        self.schemeqInputs = None;
        self.checkInputs()
        # File to store output on disk
        self.hdfFileName = "rs%5.3f_theta%5.3f_%s.h5" % (self.inputs.coupling,
                                                         self.inputs.degeneracy,
                                                         self.inputs.theory)
        # Non-default inputs
        if (fixed is not None):
            self.qInputs.fixed = fixed
        if (guess is not None):
            self.qInputs.guess = guess
        if (threads is not None):
            self.inputs.threads = threads

    # Compute
    def compute(self):
        self.checkInputs()
        self.unpackFixedAdrFiles()
        self.schemeInputs = self.inputs
        self.schemeqInputs = self.qInputs
        self.scheme = qp.Qstls(self.schemeInputs, self.schemeqInputs)
        status = self.scheme.compute()
        self.checkStatusAndClean(status)
        self.setHdfFile()
        self.save()

    # Unpack zip folder with fixed component of the auxiliary density response
    def unpackFixedAdrFiles(self):
        # this is just a hook to the corresponding method in QstlsIet
        pass
    
    # Save results to disk
    def save(self):
        super().save()
        pd.DataFrame(self.scheme.adr).to_hdf(self.hdfFileName, key="adr")
        
    # Plot results        
    def plot(self, toPlot, matsubara = None, rdfGrid = np.arange(0.01, 10.0, 0.01)):
        super().plot(toPlot, matsubara, rdfGrid)
        if ("adr" in toPlot): self.plotAdr(matsubara)

    def plotAdr(self, matsubara):
        if (matsubara is None) : matsubara = np.arange(self.inputs.matsubara)
        Plot.plot1DParametric(self.scheme.wvg, self.scheme.adr,
                              "Wave vector", "Auxiliary density response",
                              matsubara)
        

class QstlsIet(Qstls):
            
    # Constructor
    def __init__(self,
                 coupling,
                 degeneracy,
                 theory,
                 chemicalPotential = None,
                 cutoff = None,
                 error = None,
                 fixed = None,
                 fixediet = None,
                 mapping = None,
                 mixing = None,
                 guess = None,
                 iterations = None,
                 matsubara = None,
                 outputFrequency = None,
                 recoveryFile = None,
                 scheme2DIntegrals = None,
                 resolution = None,
                 threads = None):
        # Call parent constructor
        super().__init__(coupling, degeneracy,
                         chemicalPotential, cutoff, error,
                         fixed, mixing, guess, iterations,
                         matsubara, outputFrequency,
                         recoveryFile, resolution)
        # Allowed theories
        self.allowedTheories = ["QSTLS-HNC", "QSTLS-IOI", "QSTLS-LCT"]
        # Set theory
        self.inputs.theory = theory
        self.checkInputs()
        # Temporary folder to store the unpacked files with the auxiliary density response
        self.fixediet = None
        self.tmpRunDir = None
        # File to store output on disk
        self.hdfFileName = "rs%5.3f_theta%5.3f_%s.h5" % (self.inputs.coupling,
                                                           self.inputs.degeneracy,
                                                           self.inputs.theory)
        # Non-default inputs
        if (fixediet is not None):
            self.fixediet = fixediet
        if (mapping is not None):
            self.inputs.iet = mapping
        if (scheme2DIntegrals is not None):
            self.inputs.int2DScheme = scheme2DIntegrals

    # Unpack zip folder with fixed component of the auxiliary density response
    def unpackFixedAdrFiles(self):
        if (self.fixediet is not None):
            self.tmpRunDir = "qupled_tmp_run_directory"
            zipFile = zf.ZipFile(self.fixediet, "r")
            zipFile.extractall(self.tmpRunDir)
            self.qInputs.fixediet = self.tmpRunDir
    
    # Check that the dielectric scheme was solved without errors
    def checkStatusAndClean(self, status):
        if (self.fixediet is not None):
            rmtree(self.tmpRunDir)
        if (status == 0):
            if os.path.isfile(self.scheme.recovery) : os.remove(self.scheme.recovery)
            print("Dielectric theory solved successfully!")
        else:
            sys.exit("Error while solving the dielectric theory")

            
    # Save results to disk
    def save(self):
        super().save()
        pd.DataFrame(self.scheme.bf).to_hdf(self.hdfFileName, key="bf")
        # Zip all files for the fixed component of the auxiliary density response
        if (self.schemeqInputs.fixediet == ""):
            adrFileName = "adr_fixed_rs%5.3f_theta%5.3f_%s" % (self.schemeInputs.coupling,
                                                               self.schemeInputs.degeneracy,
                                                               self.schemeInputs.theory)
            zipFile = zf.ZipFile(adrFileName + ".zip", "w")
            for adrFile in glob(adrFileName + "_wv*.bin"):
                zipFile.write(adrFile)
                os.remove(adrFile)

                
    # Plot results        
    def plot(self, toPlot, matsubara = None, rdfGrid = np.arange(0.01, 10.0, 0.01)):
        super().plot(toPlot, matsubara, rdfGrid)
        if ("bf" in toPlot):
            Plot.plot1D(self.scheme.wvg, self.scheme.bf, "Wave vector", "Bridge function adder")
        


class Hdf():

    # Plot from data in hdf file
    def plot(hdf, toPlot, matsubara = None, rdfGrid = np.arange(0.01, 10.0, 0.01)):
        if (matsubara is None) : matsubara = np.arange(pd.read_hdf(hdf, "inputs")["matsubara"][0].tolist())
        wvg = pd.read_hdf(hdf, "wvg")[0].to_numpy()
        xlabel = "Wave vector"
        if ("adr" in toPlot):
            adr = pd.read_hdf(hdf, "adr").to_numpy()
            Plot.plot1DParametric(wvg, adr, xlabel, "Auxiliary density response", matsubara)
        if ("bf" in toPlot):
            bf = pd.read_hdf(hdf, "bf")[0].to_numpy()
            Plot.plot1D(wvg, bf, xlabel, "Bridge function adder")
        if ("idr" in toPlot):
            idr = pd.read_hdf(hdf, "idr").to_numpy()
            Plot.plot1DParametric(wvg, idr, xlabel, "Ideal density response", matsubara)
        if ("rdf" in toPlot):
            rdf = pd.read_hdf(hdf, "rdf")[0].to_numpy()
            spg = pd.read_hdf(hdf, "rdfGrid")[0].to_numpy()
            Plot.plot1D(spg, rdf, "Inter-particle distance", "Radial distribution function")
        if ("sdr" in toPlot):
            sdr = pd.read_hdf(hdf, "sdr")[0].to_numpy()
            Plot.plot1D(wvg, sdr, xlabel, "Static density response")
        if ("slfc" in toPlot):
            slfc = pd.read_hdf(hdf, "slfc")[0].to_numpy()
            Plot.plot1D(wvg, slfc, xlabel, "Static density response")
        if ("ssf" in toPlot):
            ssf = pd.read_hdf(hdf, "ssf")[0].to_numpy()
            Plot.plot1D(wvg, ssf, xlabel, "Static structure factor")
        if ("ssfHF" in toPlot):
            ssfHF = pd.read_hdf(hdf, "ssfHF")[0].to_numpy()
            Plot.plot1D(wvg, ssfHF, xlabel, "Hartree-Fock static structure factor")

    def computeRdf(hdf, rdfGrid = np.arange(0.01, 10.0, 0.01), saveRdf=True):
        wvg = pd.read_hdf(hdf, "wvg")[0].to_numpy()
        ssf = pd.read_hdf(hdf, "ssf")[0].to_numpy()
        rdf = qp.computeRdf(rdfGrid, wvg, ssf)
        if (saveRdf):
            pd.DataFrame(rdfGrid).to_hdf(hdf, key="rdfGrid", mode="r+")
            pd.DataFrame(rdf).to_hdf(hdf, key="rdf", mode="r+")
        return rdf

    def computeInternalEnergy(hdf):
        wvg = pd.read_hdf(hdf, "wvg")[0].to_numpy()
        ssf = pd.read_hdf(hdf, "ssf")[0].to_numpy()
        coupling = pd.read_hdf(hdf, "inputs")["coupling"][0].tolist()
        return qp.computeInternalEnergy(wvg, ssf, coupling)

    
class Plot():
    
    # One dimensional plots
    def plot1D(x, y, xlabel, ylabel):
        plt.plot(x, y, "b")
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.show()        
    
    # One dimensional plots with one parameter
    def plot1DParametric(x, y, xlabel, ylabel, parameters):
        numParameters = parameters.size
        cmap = cm.get_cmap(name="viridis")
        for i in np.arange(numParameters):
            color = cmap(1.0*i/numParameters)
            plt.plot(x, y[:,parameters[i]], color=color)
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.show()        
