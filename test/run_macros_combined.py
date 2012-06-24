import os

type       = "rescaled" 

channels   = [
    "emu",
    "eTau",
    "muTau",
    ]

categories = [
    "0jet_low",
    "0jet_high",
    "0jet",
    "boost_low",
    "boost_high",
    "boost",
    "vbf",
    ] 

log = {
    ("emu"  , "0jet_low"  ) : "true",
    ("emu"  , "0jet_high" ) : "true",
    ("emu"  , "0jet"      ) : "true", 
    ("emu"  , "boost_low" ) : "false",
    ("emu"  , "boost_high") : "false",
    ("emu"  , "boost"     ) : "false",
    ("emu"  , "vbf"       ) : "false",
    ("muTau", "0jet_low"  ) : "true",
    ("muTau", "0jet_high" ) : "true",
    ("muTau", "0jet"      ) : "true", 
    ("muTau", "boost_low" ) : "false",
    ("muTau", "boost_high") : "false",
    ("muTau", "boost"     ) : "false",
    ("muTau", "vbf"       ) : "false",
    ("eTau" , "0jet_low"  ) : "true",
    ("eTau" , "0jet_high" ) : "true",
    ("eTau" , "0jet"      ) : "true", 
    ("eTau" , "boost_low" ) : "false",
    ("eTau" , "boost_high") : "false",
    ("eTau" , "boost"     ) : "false",
    ("eTau" , "vbf"       ) : "false",    
    }

max = {
    ("emu"  , "0jet_low"  ) :  "700",
    ("emu"  , "0jet_high" ) :  "100", 
    ("emu"  , "0jet"      ) :  "700",
    ("emu"  , "boost_low" ) :   "80",
    ("emu"  , "boost_high") :   "20",
    ("emu"  , "boost"     ) :  "100",
    ("emu"  , "vbf"       ) :  "1.2",
    ("muTau", "0jet_low"  ) : "3000",
    ("muTau", "0jet_high" ) :  "500",
    ("muTau", "0jet"      ) : "3000", 
    ("muTau", "boost_low" ) :  "300",
    ("muTau", "boost_high") :  "100",
    ("muTau", "boost"     ) :  "400",
    ("muTau", "vbf"       ) :  "4.0",
    ("eTau" , "0jet_low"  ) :  "200",
    ("eTau" , "0jet_high" ) :  "100",
    ("eTau" , "0jet"      ) :  "200", 
    ("eTau" , "boost_low" ) :   "60",
    ("eTau" , "boost_high") :   "20",
    ("eTau" , "boost"     ) :   "80",
    ("eTau" , "vbf"       ) :  "2.2",    
    }

min = {
    ("emu"  , "0jet_low"  ) : "0.1",
    ("emu"  , "0jet_high" ) : "0.1",
    ("emu"  , "0jet"      ) : "0.1", 
    ("emu"  , "boost_low" ) : "0",
    ("emu"  , "boost_high") : "0",
    ("emu"  , "boost"     ) : "0",
    ("emu"  , "vbf"       ) : "0",
    ("muTau", "0jet_low"  ) : "0.5",
    ("muTau", "0jet_high" ) : "0.1",
    ("muTau", "0jet"      ) : "0.5", 
    ("muTau", "boost_low" ) : "0",
    ("muTau", "boost_high") : "0",
    ("muTau", "boost"     ) : "0",
    ("muTau", "vbf"       ) : "0",
    ("eTau" , "0jet_low"  ) : "0.3",
    ("eTau" , "0jet_high" ) : "0.1",
    ("eTau" , "0jet"      ) : "0.3", 
    ("eTau" , "boost_low" ) : "0",
    ("eTau" , "boost_high") : "0",
    ("eTau" , "boost"     ) : "0",
    ("eTau" , "vbf"       ) : "0",    
    }


## combine 2011+2012
#for chn in channels :
#    for cat in categories :
#        ## combine high and low pt categories, make sure in your 
#        ## list that {CAT}_low and {CAT}_high are run beforehand
#        if cat == "0jet" or cat == "boost" :
#            os.system("hadd {CHN}_{CAT}_{TYPE}_7+8TeV.root {CHN}_{CAT}_low_{TYPE}_7+8TeV.root {CHN}_{CAT}_high_{TYPE}_7+8TeV.root".format(CHN=chn, CAT=cat, TYPE=type))
#        else :
#            ## patch until Josh fixes his naming conventions
#            if chn == "eTau" : 
#                os.system("hadd {CHN}_{CAT}_{TYPE}_7+8TeV.root {CHN}_{CAT}_{TYPE}_7TeV_{LOG}.root eleTau_{CAT}_{TYPE}_8TeV_{LOG}.root".format(
#                    CHN=chn, CAT=cat, TYPE=type, LOG="LOG" if "0jet" in cat else ""))
#            else:
#                os.system("hadd {CHN}_{CAT}_{TYPE}_7+8TeV.root {CHN}_{CAT}_{TYPE}_7TeV_{LOG}.root {CHN}_{CAT}_{TYPE}_8TeV_{LOG}.root".format(
#                    CHN=chn, CAT=cat, TYPE=type, LOG="LOG" if "0jet" in cat else ""))
## make plots
for chn in channels :
    for cat in categories :
        print chn, cat
        os.system("root -l -q -b {CMSSW_BASE}/src/HiggsAnalysis/HiggsToTauTau/macros/postfit.C+\\(\\\"{CHN}_{CAT}_{TYPE}_7+8TeV.root\\\",\\\"{LABEL}\\\",\\\"{EXTRA}\\\",{MIN},{MAX},{LOG}\)".format(CMSSW_BASE=os.environ['CMSSW_BASE'], CHN=chn, CAT=cat, TYPE=type, LABEL="2011+2012", EXTRA="", MIN=min[chn,cat], MAX=max[chn,cat], LOG=log[chn,cat]))
        