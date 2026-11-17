# Research: Morning Stockout & Reorder Helper for a Neighborhood Kirana Store

## The Problem
Every morning within walking distance of my home, the neighborhood kirana store handles a fast rush of small daily-essential purchases—milk packets, bread, eggs, curd, tea sachets, and loose staples. Because a single shopkeeper manages billing, cash/UPI, and bagging simultaneously, they have no time to log sales and must guess the next day's morning order from memory. As a result, fast-moving perishables like half-liter milk packets or ₹10 bread loaves sell out before 9:30 AM on busy mornings, losing sales and frustrating regular neighbors. On slower mornings, over-ordered perishables sit unsold and spoil. A lightweight tool that turns 20–30 minutes of morning observation into concrete reorder quantities and rush-hour insights can help the shopkeeper cut both stockouts and waste.

## Target Person & Location
- **Person:** [Rajesh Kumar, Shopkeeper]
- **Place:** [Rajesh general store,Amaithi,Rohtas,Bihar,802220]

## 2 Existing Solutions & Why They Do Not Fit
1. **Full Retail POS & Billing Software (e.g., Vyapar, Marg ERP, Zoho POS):**
   These tools require barcode scanners, upfront catalog entry for hundreds of SKUs, and per-item billing clicks during checkout. For a one-person kirana counter where 95% of transactions are small ₹10–₹200 cash/UPI purchases during a crowded morning rush, stopping to tap a screen for every packet of milk slows down the line and never gets used consistently.
2. **Digital Khata / Ledger Apps (e.g., Khatabook, OkCredit):**
   While popular for tracking customer *udhaar* (credit) balances and sending WhatsApp payment reminders, ledger apps track *who owes money* rather than *item-level morning stock velocity or stockout events*. They do not tell the shopkeeper how many extra milk packets or bread loaves to order for tomorrow morning.

## 2 Sources Read & Notes
1. **[Why Small Shop Owners in India Never Actually Use an Inventory App — Synline](https://www.synline.co.in/blog/why-small-shops-dont-use-inventory-apps.html)**
   - Inventory apps fail small Indian retailers not because owners dislike technology, but because every app assumes an uninterrupted hour to type in an entire SKU catalog upfront.
   - For a solo shopkeeper interrupted every minute by walk-in customers, that setup cost never gets paid back fast enough.
   - This confirms my tool must require zero live catalog entry by the shopkeeper and instead work offline from a simple CSV log.

2. **[Modernization of Kirana Stores in India — Invest India](https://www.investindia.gov.in/team-india-blogs/modernization-kirana-stores-india)**
   - Around 98% of India's kirana stores are operated and managed by a single person, and 95% of transactions are below ₹200.
   - Limited visibility into daily inventory movement frequently leads to both unexpected stockouts and damaged or expired perishable goods.
   - Designing specifically for a solo operator handling high-frequency, low-ticket morning essentials directly addresses this bottleneck.