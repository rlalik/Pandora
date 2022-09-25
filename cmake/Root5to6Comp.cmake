foreach(
  _expectedTarget
  Cint
  Reflex
  genmap
  Cintex
  Rint
  Thread
  New
  rootcint
  rlibmap
  Core
  rmkdepend
  MathCore
  MathMore
  Matrix
  Minuit
  Minuit2
  Fumili
  Physics
  MLP
  Quadp
  Foam
  Smatrix
  SPlot
  GenVector
  Genetic
  FFTW
  Hist
  HistPainter
  Spectrum
  SpectrumPainter
  Hbook
  Tree
  TreePlayer
  TreeViewer
  RIO
  SQLIO
  XMLIO
  XMLParser
  Net
  RootAuth
  RDAVIX
  Gpad
  Graf
  Postscript
  mathtext
  GX11
  GX11TTF
  ASImage
  ASImageGui
  Gviz
  Graf3d
  X3d
  Eve
  RGL
  Gviz3d
  Gui
  Ged
  FitPanel
  GuiBld
  GuiHtml
  Recorder
  SessionViewer
  Proof
  ProofPlayer
  ProofDraw
  ProofBench
  Html
  EG
  VMC
  Geom
  GeomBuilder
  GeomPainter
  root
  minicern
  rootn.exe
  roots.exe
  ssh2rpd
  root.exe
  proofserv.exe
  hadd
  g2root
  h2root
  TMVA)
  if(TARGET ${_expectedTarget})
    get_target_property(target_type ${_expectedTarget} TYPE)
    if(NOT target_type STREQUAL "EXECUTABLE")
      if(${CMAKE_VERSION} VERSION_LESS "3.18.0")
        set_target_properties(${_expectedTarget} PROPERTIES IMPORTED_GLOBAL
                                                            TRUE)
      endif()
      add_library(ROOT::${_expectedTarget} ALIAS ${_expectedTarget})
    endif()
  endif()
endforeach()
