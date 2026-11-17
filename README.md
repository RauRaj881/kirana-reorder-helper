# Kirana Morning Rush & Reorder Helper

- **Place:** [Rajesh general store,amaithi,bihar,802220]
- **Person Helped:** [Rajesh Kumar,shopkeeper]
- **Date:** [2026-11-17]
## What This Tool Does
Reads real morning transaction observations from `data.csv` and outputs item-level reorder quantities (with a configurable safety buffer and stockout bump) plus the peak 30-minute rush window.

## How to Run
```bash
g++ -std=c++17 -O2 main.cpp -o reorder
./reorder data.csv
