# Known Issues

## Mount Type Display

**Issue:** INDI clients (KStars/Ekos) show the alpaca as a German Equatorial Mount (GEM) by default, with meridian flip options visible but greyed out.

**Root Cause:** The alpaca Alpaca API does not implement the pier side methods (returns error 1024 for `/sideofpier`). INDI assumes GEM mount type when pier side information is not available.

**Workaround:** Manually configure the mount type in KStars/Ekos:
1. Open Ekos Mount module
2. Go to mount options/settings
3. Change "Mount Type" from "German Equatorial" to "Fork" or "Alt-Az"
4. This will disable meridian flip logic

**Status:** This is a limitation of how INDI determines mount type. The driver correctly reports that pier side is not available, but clients interpret this as GEM rather than Fork/Alt-Az.

---

## GoTo Requires Telescope Initialization

**Issue:** GoTo commands fail with Alpaca error 1032 when the telescope is not properly initialized.

**Root Cause:** The alpaca must be physically opened (arm deployed) and initialized before accepting slew commands. Error 1032 indicates invalid state rather than invalid coordinates.

**Workaround:** 
1. Open the alpaca telescope arm through the mobile app
2. Allow the telescope to complete initialization
3. Then connect via INDI and issue GoTo commands

**Status:** The driver now checks for "at home" position and provides clearer error messages. This is expected alpaca behavior.
