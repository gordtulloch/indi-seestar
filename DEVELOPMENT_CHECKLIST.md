# INDI Seestar Driver - Development Checklist

This document provides a comprehensive checklist for implementing a complete INDI driver for the Seestar telescope with all supported Alpaca API capabilities.

**Target**: Single multi-device INDI driver supporting Telescope, Camera, FilterWheel, and Focuser  
**API**: ASCOM Alpaca REST API v1  
**Firmware**: Seestar v1.1.2-1  
**Reference**: [AlpacaAPIComparison.md](AlpacaAPIComparison.md)

---

## Project Setup & Infrastructure

### Development Environment
- [x] Create INDI driver project structure
- [x] Set up CMake build system for INDI driver
- [x] Configure INDI library dependencies
- [x] Set up libcurl for HTTP/REST communication
- [ ] Add JSON parsing library (nlohmann/json or similar) - Using simple parser for now
- [x] Create driver XML configuration file
- [x] Set up logging and debugging infrastructure (INDI built-in)
- [ ] Create unit test framework
- [x] Document build and installation process

### Code Organization
- [x] Design class hierarchy for multi-device driver
- [x] Implement base Alpaca communication class
- [x] Create HTTP client wrapper for REST calls
- [x] Implement JSON response parser (simple version)
- [x] Create error handling and mapping (ASCOM → INDI)
- [x] Design configuration storage system
- [ ] Implement device discovery mechanism (manual config for now)

---

## Common Functionality (All Devices)

### Connection Management
- [x] Implement device discovery (UDP broadcast on port 32227) - Manual config for v1
- [x] Implement connection establishment (`/connected` PUT)
- [x] Implement connection state monitoring (`/connected` GET)
- [x] Implement graceful disconnection
- [x] Add connection timeout handling
- [ ] Implement reconnection logic

### Device Information
- [x] Query and display device description (`/description`)
- [x] Query and display driver info (`/driverinfo`)
- [x] Query and display driver version (`/driverversion`)
- [x] Query and display interface version (`/interfaceversion`)
- [ ] Query and display device name (`/name`)
- [ ] Store and validate supported actions (`/supportedactions`)

### Error Handling
- [x] Map ASCOM error codes to INDI error codes
  - [x] Error 1024: Not Implemented
  - [x] Error 1025: Invalid Value
  - [x] Error 1026: Value Not Set
  - [x] Error 1031: Invalid Operation
  - [x] Error 1279: Invalid While Parked/State
  - [x] Error 1036: Action Not Implemented
- [x] Implement user-friendly error messages
- [x] Add error logging and diagnostics

---

## Telescope Interface (INDI::Telescope)

### Basic Telescope Properties
- [x] Implement INDI::Telescope base class
- [x] Configure telescope as equatorial mount
- [x] Set up coordinate system (J2000)
- [x] Implement telescope info tab
- [x] Add firmware version display

### Position Properties
- [x] Implement RA/Dec position reporting (`/rightascension`, `/declination`)
- [x] Implement Alt/Az position reporting (`/altitude`, `/azimuth`)
- [x] Implement pier side reporting (`/sideofpier`)
- [x] Update position at regular intervals (polling)
- [ ] Convert between coordinate systems as needed

### Site Information
- [x] Read site latitude (`/sitelatitude`)
- [x] Read site longitude (`/sitelongitude`)
- [x] Read site elevation (`/siteelevation`)
- [x] Display site information (read-only)
- [x] Read and display sidereal time (`/siderealtime`)
- [x] Read and display UTC date (`/utcdate`)

### Slewing & GoTo
- [x] Implement slew to coordinates (`/slewtocoordinates`)
- [x] Implement async slew to coordinates (`/slewtocoordinatesasync`)
- [ ] Implement slew to target (`/slewtotarget`)
- [ ] Implement async slew to target (`/slewtotargetasync`)
- [ ] Set target RA (`/targetrightascension` PUT)
- [ ] Set target Dec (`/targetdeclination` PUT)
- [x] Implement slewing state monitoring (`/slewing`)
- [x] Implement abort slew (`/abortslew`)
- [ ] Add slew rate controls if applicable
- [x] **Note**: Skip Alt/Az slewing (not supported - error 1024)

### Parking
- [x] Implement park command (`/park`)
- [x] Implement unpark command (`/unpark`)
- [x] Implement park status (`/atpark`)
- [x] Add park position indicator
- [x] Handle park-dependent operation errors (1279)
- [x] **Note**: Park position not settable (`/setpark` not implemented)

### Tracking
- [x] Implement tracking enable/disable (`/tracking` PUT)
- [x] Implement tracking state query (`/tracking` GET)
- [x] Implement tracking rate selection (`/trackingrate`)
  - [x] Sidereal (0)
  - [x] Lunar (1)
  - [x] Solar (2)
- [ ] Query available tracking rates (`/trackingrates`)
- [x] Display current tracking rate

### Sync & Alignment
- [x] Implement sync to coordinates (`/synctocoordinates`)
- [ ] Implement sync to target (`/synctotarget`)
- [ ] Handle sync state dependencies
- [ ] Add alignment point management (INDI side)
- [x] **Note**: Skip Alt/Az sync (not supported)

### Home Position
- [x] Implement find home (`/findhome`)
- [x] Implement home status query (`/athome`)
- [x] Handle home-finding state dependencies (fails when parked)
- [x] Add home position indicator

### Manual Motion
- [ ] Implement axis rates query (`/axisrates`)
- [x] Implement move axis (`/moveaxis`)
  - [x] Primary axis (RA) - TelescopeAxes=0
  - [x] Secondary axis (Dec) - TelescopeAxes=1
- [x] Implement motion controls (N/S/E/W buttons)
- [ ] Add variable speed controls
- [ ] Implement stop motion

### Guide/Pulse Commands
- [ ] Implement pulse guide (`/pulseguide`)
- [ ] Handle pulse guide directions (N/S/E/W)
- [ ] Query guide rates (`/guideratedeclination`, `/guideraterightascension`)
- [ ] Add guide rate display (read-only, cannot set)
- [ ] Handle tracking requirement for pulse guide

### Capabilities
- [ ] Query and report all telescope capabilities
  - [ ] canfindhome: true
  - [ ] canpark: true
  - [ ] canpulseguide: true
  - [ ] cansettracking: true
  - [ ] canslew: true
  - [ ] canslewasync: true
  - [ ] cansync: true
  - [ ] canunpark: true
- [ ] Report unsupported capabilities
  - [ ] canslewaltaz: false
  - [ ] cansetpark: false
  - [ ] cansetpierside: false

---

## Camera Interface (INDI::CCD)

### Basic Camera Properties
- [ ] Implement INDI::CCD base class
- [ ] Set camera resolution (1080x1920)
- [ ] Set pixel size (2.9µm x 2.9µm)
- [ ] Configure Bayer pattern (GRBG, offsets X=1, Y=0)
- [ ] Set bit depth (16-bit, max ADU 65535)
- [ ] Display sensor type (Color/Bayer)
- [ ] Query sensor name (`/sensorname`)

### Exposure Control
- [ ] Implement exposure start (`/startexposure` PUT)
- [ ] Set exposure duration parameter
- [ ] Query exposure limits
  - [ ] Min: 0.00003s (30µs)
  - [ ] Max: 2000s (33.3 minutes)
  - [ ] Resolution: 1µs
- [ ] Implement exposure state monitoring (`/camerastate`)
  - [ ] 0: Idle
  - [ ] 1: Waiting
  - [ ] 2: Exposing
  - [ ] 3: Reading
  - [ ] 4: Download
  - [ ] 5: Error
- [ ] Query image ready status (`/imageready`)
- [ ] Implement exposure abort (`/abortexposure`)
- [ ] Implement exposure stop (`/stopexposure`)
- [ ] Add exposure progress indicator

### Image Download
- [ ] Implement image array download (`/imagearray`)
- [ ] Implement variant image array (`/imagearrayvariant`)
- [ ] Parse image data format
- [ ] Convert to INDI image format
- [ ] Apply Bayer pattern metadata
- [ ] Handle 16-bit data properly
- [ ] Implement image transfer to client

### Binning
- [ ] Report binning capabilities
  - [ ] Max X: 1 (no binning)
  - [ ] Max Y: 1 (no binning)
  - [ ] Asymmetric: false
- [ ] Query current binning (`/binx`, `/biny`)
- [ ] Display binning as 1x1 only
- [ ] Disable binning controls in UI

### Subframe/ROI
- [ ] Implement subframe start X (`/startx` PUT)
- [ ] Implement subframe start Y (`/starty` PUT)
- [ ] Implement subframe width (`/numx` PUT)
- [ ] Implement subframe height (`/numy` PUT)
- [ ] Query current subframe settings (GET)
- [ ] Validate subframe bounds (0,0 to 1080x1920)
- [ ] Add full frame reset option

### Gain Control
- [ ] Query gain range (0-400)
- [ ] Implement gain setting (`/gain` PUT)
- [ ] Query current gain (`/gain` GET)
- [ ] Query gain min/max (`/gainmin`, `/gainmax`)
- [ ] Add gain slider to UI
- [ ] **Note**: Discrete gains list not available (`/gains` error 1024)

### Offset Control
- [ ] Report offset as not available
- [ ] Disable offset controls
- [ ] **Note**: All offset methods return error 1024

### Readout Modes
- [ ] Query available readout modes (`/readoutmodes`)
- [ ] Display single mode (mode 0)
- [ ] Query current mode (`/readoutmode`)
- [ ] **Note**: Only one readout mode available

### Temperature Monitoring
- [ ] Query CCD temperature (`/ccdtemperature`)
- [ ] Display temperature (read-only)
- [ ] Add temperature to FITS header
- [ ] Poll temperature at regular intervals
- [ ] **Note**: No cooling control (all cooling methods error 1024)

### Capabilities
- [ ] Report camera capabilities
  - [ ] canabortexposure: true
  - [ ] canstopexposure: true
- [ ] Report unsupported capabilities
  - [ ] canasymmetricbin: false
  - [ ] canfastreadout: false
  - [ ] cangetcoolerpower: false
  - [ ] canpulseguide: false
  - [ ] cansetccdtemperature: false
  - [ ] hasshutter: false

---

## FilterWheel Interface (INDI::FilterWheel)

### Basic FilterWheel Properties
- [ ] Implement INDI::FilterWheel base class
- [ ] Set filter count (3 filters)
- [ ] Query filter names (`/names`)
  - [ ] Position 0: "Dark"
  - [ ] Position 1: "IR"
  - [ ] Position 2: "LP"
- [ ] Display filter names in UI

### Position Control
- [ ] Query current position (`/position` GET)
- [ ] Implement position setting (`/position` PUT)
- [ ] Validate position range (0-2)
- [ ] Handle position change requests
- [ ] Add position indicator to UI
- [ ] Prevent redundant moves (error 1279)

### Focus Offsets
- [ ] Query focus offsets (`/focusoffsets`)
- [ ] Store offset values ([0,0,0] initially)
- [ ] Implement filter change with focus adjustment
- [ ] Coordinate with focuser for offset application
- [ ] Add offset calibration tool (optional)

### Filter Management
- [ ] Allow filter name customization (INDI side)
- [ ] Store filter configuration
- [ ] Add filter selection dropdown
- [ ] Display current filter name
- [ ] Log filter changes

---

## Focuser Interface (INDI::Focuser)

### Basic Focuser Properties
- [ ] Implement INDI::Focuser base class
- [ ] Set focuser type as absolute
- [ ] Set position range (0-2600 steps)
- [ ] Query maximum step (`/maxstep`)
- [ ] Query maximum increment (`/maxincrement`)
- [ ] Display focuser info
- [ ] **Note**: Step size not available (error 1024)

### Position Control
- [ ] Query current position (`/position` GET)
- [ ] Implement absolute move (`/move` PUT with absolute position)
- [ ] **Implement relative move calculation** (driver side)
  - [ ] Query current position
  - [ ] Calculate: target = current + offset
  - [ ] Validate: 0 <= target <= 2600
  - [ ] Send absolute move command
- [ ] Query movement state (`/ismoving`)
- [ ] Implement halt command (`/halt`)
- [ ] Add position indicator
- [ ] Add movement progress indicator

### Movement Handling
- [ ] Validate position range before moves
- [ ] Handle error 1025 (invalid value for relative)
- [ ] Poll ismoving status during movement
- [ ] Implement movement timeout
- [ ] Add movement speed estimation
- [ ] Queue movement commands appropriately

### Temperature
- [ ] Query temperature (`/temperature`)
- [ ] Display temperature (read-only)
- [ ] Query temp compensation state (`/tempcomp` GET)
- [ ] Display compensation as unavailable
- [ ] **Note**: Cannot set temp compensation (error 1024)

### Capabilities
- [ ] Report absolute positioning: true
- [ ] Report temperature compensation: false
- [ ] Disable temp compensation controls
- [ ] Add manual temperature monitoring display

---

## Integration & Coordination

### Multi-Device Coordination
- [ ] Implement filter change with focus offset
- [ ] Coordinate camera exposure with telescope tracking
- [ ] Handle focus adjustments during imaging
- [ ] Synchronize device states
- [ ] Implement device priority for shared resources

### State Management
- [ ] Track connection state for all devices
- [ ] Manage park/unpark state and dependencies
- [ ] Track tracking state for pulse guide
- [ ] Monitor camera state for exposure readiness
- [ ] Store filter and focus positions
- [ ] Implement state persistence across connections

### Configuration
- [ ] Create driver configuration dialog
- [ ] Add network settings (hostname/IP, port)
- [ ] Add device enable/disable options
- [ ] Store connection preferences
- [ ] Add polling interval settings
- [ ] Create configuration file format
- [ ] Implement configuration save/load

---

## User Interface

### INDI Properties
- [ ] Define all INDI properties for telescope
- [ ] Define all INDI properties for camera
- [ ] Define all INDI properties for filterwheel
- [ ] Define all INDI properties for focuser
- [ ] Organize properties into logical groups
- [ ] Add appropriate property permissions (RO/RW)
- [ ] Set property update policies

### Client Compatibility
- [ ] Test with KStars/Ekos
- [ ] Test with INDI Control Panel
- [ ] Test with PHD2 guiding
- [ ] Test with CCDciel
- [ ] Verify FITS header metadata
- [ ] Test image preview and download
- [ ] Verify coordinate system conversions

---

## Testing & Validation

### Unit Tests
- [ ] Test HTTP/REST communication
- [ ] Test JSON parsing
- [ ] Test error code mapping
- [ ] Test coordinate conversions
- [ ] Test relative-to-absolute position calculations
- [ ] Test state machine logic

### Integration Tests
- [ ] Test connection/disconnection cycle
- [ ] Test telescope slewing and tracking
- [ ] Test camera exposure sequence
- [ ] Test filter wheel operation
- [ ] Test focuser movement
- [ ] Test abort/stop commands
- [ ] Test error recovery

### Functional Tests
- [ ] Perform plate solving sequence
- [ ] Perform focus run
- [ ] Capture image sequence with filter changes
- [ ] Test guiding with pulse guide
- [ ] Test parking and home operations
- [ ] Simulate network failures
- [ ] Test concurrent operations

### Performance Tests
- [ ] Measure polling overhead
- [ ] Test image download speed
- [ ] Measure command response times
- [ ] Test multiple client connections
- [ ] Monitor memory usage
- [ ] Check for resource leaks

---

## Documentation

### Code Documentation
- [ ] Document all classes and methods
- [ ] Add inline comments for complex logic
- [ ] Document error handling strategies
- [ ] Create architecture overview document
- [ ] Document state machines
- [ ] Add API usage examples

### User Documentation
- [ ] Write installation instructions
- [ ] Create configuration guide
- [ ] Document known limitations
- [ ] Create troubleshooting guide
- [ ] Add usage examples
- [ ] Document compatibility requirements
- [ ] Create FAQ section

### Developer Documentation
- [ ] Document build process
- [ ] Create contribution guidelines
- [ ] Document testing procedures
- [ ] Add debugging tips
- [ ] Create release process document

---

## Deployment & Release

### Packaging
- [ ] Create distribution packages
  - [ ] Debian/Ubuntu (.deb)
  - [ ] RPM-based distributions
  - [ ] Source tarball
- [ ] Create installation scripts
- [ ] Add to INDI 3rd party repository
- [ ] Create Docker container (optional)

### Release Management
- [ ] Define version numbering scheme
- [ ] Create release checklist
- [ ] Set up CI/CD pipeline
- [ ] Create release notes template
- [ ] Tag releases in git
- [ ] Publish release announcements

### Support & Maintenance
- [ ] Set up issue tracking
- [ ] Create support channel (forum/Discord)
- [ ] Monitor firmware updates (Seestar)
- [ ] Plan for INDI API changes
- [ ] Schedule regular testing
- [ ] Plan for feature requests

---

## Known Limitations to Document

### Telescope
- No Alt/Az slewing support (RA/Dec only)
- Site information is read-only
- Guide rates are read-only
- Park position cannot be set
- Pier side information limited

### Camera
- No binning support (1x1 only)
- No offset control
- No active cooling control
- No pulse guiding via camera
- Temperature monitoring only

### FilterWheel
- Focus offsets need manual calibration
- Three filters only (fixed)

### Focuser
- **No relative positioning** (must calculate absolute)
- No step size information (unitless steps)
- No temperature compensation
- Temperature monitoring only

---

## Future Enhancements (Post-V1)

- [ ] Add autoguiding support using telescope pulse guide
- [ ] Implement focus calibration routine
- [ ] Add filter offset calibration wizard
- [ ] Create meridian flip handling
- [ ] Add polar alignment assistance
- [ ] Implement image stacking (driver or client side)
- [ ] Add flat frame capture automation
- [ ] Create focus graph analysis
- [ ] Add exposure calculator
- [ ] Implement batch sequence support
- [ ] Add weather monitoring integration
- [ ] Create mobile app configuration interface

---

## Progress Tracking

**Phase 1: Foundation** (Setup & Common)
- Status: ✅ **COMPLETE**
- Completion: January 2, 2026

**Phase 2: Telescope** (Primary device)
- Status: 🔄 **IN PROGRESS** (70% complete)
- Started: January 2, 2026
- Target: January 23, 2026

**Phase 3: Camera** (Imaging capability)
- Status: Not Started
- Target: 3 weeks

**Phase 4: FilterWheel & Focuser** (Accessories)
- Status: Not Started
- Target: 2 weeks

**Phase 5: Integration & Testing**
- Status: Not Started
- Target: 2 weeks

**Phase 6: Documentation & Release**
- Status: Not Started
- Target: 1 week

**Total Estimated Time**: 13 weeks
**Current Progress**: ~15% overall

---

## Notes

- All testing completed: [AlpacaAPIComparison.md](AlpacaAPIComparison.md)
- Test programs available in `tests/` directory
- Build system already established for tests
- Focus on absolute positioning workaround for focuser
- Remember error 1279 for park-dependent operations
- FilterWheel has 100% implementation - easiest to start
- Consider test-driven development approach throughout

---

**Last Updated**: January 2, 2026  
**Status**: Ready for development
