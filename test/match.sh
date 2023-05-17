#!/bin/bash
xboard -fcp "./flybyknight" -scp flybyknight0 -smpCores 4 -matchGames 2 
xboard -fcp "./flybyknight" -scp flybyknight0 -smpCores 4 -matchGames 2 -loadPositionFile ../test/positions/KRRk.fen
xboard -fcp "./flybyknight" -scp flybyknight0 -smpCores 4 -matchGames 2 -loadPositionFile ../test/positions/italian.fen