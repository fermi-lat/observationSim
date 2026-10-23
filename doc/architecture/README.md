---
package: observationSim
summary: The gtobssim tool and its library, which simulate LAT observations by drawing photons from flux/celestialSources XML source models, passing them through the IRFs (effective area, PSF, energy dispersion), and writing FT1 and FT2 files.
layer: analysis
depends_on: [flux, celestialSources, irfs, dataSubselector, fitsGen, astro, tip, st_app, st_stream, st_facilities, facilities, hoops, CLHEP]
used_by: []
last_reviewed_commit: e4965eb
---

# observationSim Architecture

**observationSim** is the Monte Carlo observation simulator behind
**`gtobssim`**. It takes photon sources defined in `flux`-style XML (the
source classes come from `flux` and `celestialSources`), a spacecraft orbit
and attitude (simulated by `astro::GPS`, or read from an FT2 file), and a set
of LAT IRFs. It writes simulated FT1 event files, FT2 spacecraft files
(when it simulates the orbit) and a per-source summary text file.

The Doxygen mainpage (`src/mainpage.h`) describes the 2003–2004 `obsSim`
program: its `obsSim.par`, the Glast25 and DC1 IRFs, and CVS links. All of
it is stale. Use `pfiles/gtobssim.par` and `src/obsSim/obsSim.cxx` as the
source of truth. `doc/release.notes` stops at observationSim-10-01-02
(April 2015).

## Purpose and scope

**Does:**
- Load XML source libraries, select the sources named in `srclist`, and
  merge them into one time-ordered photon stream (`flux::FluxMgr`,
  `CompositeSource`).
- Drive the spacecraft through `astro::GPS`: either a rocking strategy
  (default: once per orbit at 35°, overridable with `rockangle`), or a
  pointing history from an FT2 file.
- Thin the incident photons by effective area × livetime fraction, drop
  photons during SAA passages, choose an IRF (front/back, or PSF/EDISP
  event types), and sample the apparent direction (PSF) and energy (energy
  dispersion).
- Apply the user's energy range, acceptance cone and IRF-implied cuts with
  `dataSubselector::Cuts`, and record them as DSS keywords.
- Write FT1 (`<evroot>_events_NNNN.fits`) and, if it simulated the orbit,
  FT2 (`<evroot>_scData_NNNN.fits`) with `fitsGen`. Also write
  `<evroot>_srcIds.txt`.

**Does not:**
- Model sources. Spectral and spatial models, light curves, pulsars and GRBs
  live in `flux` and `celestialSources`.
- Simulate the detector or backgrounds at the event level. It applies
  parameterized IRFs only. Residual cosmic-ray background is simulated only
  if a source in the XML provides it.
- Simulate deadtime properly. It uses a constant (or FT2-supplied)
  livetime fraction, plus a 26 µs minimum spacing between consecutive
  accepted events.
- Produce realistic GTIs. The FT1 file gets one GTI covering the whole
  simulation (see gotchas).

## Position in the system

| Direction | Package | Why |
|-----------|---------|-----|
| Depends on | `flux` | `FluxMgr` (XML source libraries, rocking setup, `pass(dt)`), `CompositeSource`, `EventSource` (time, energy, `launchDir`, `totalArea`, `applyEdisp`), `Spectrum::setStartTime`. `Simulator.h` exposes `flux/FluxMgr.h`. |
| Depends on | `celestialSources` | `gtobssim` only. `SpectrumFactoryLoader` registers the source classes (`Pulsar`, `MapCube`, `TransientTemplate`, …) so that `FluxMgr` can create them. Linked to the executables, not the library. |
| Depends on | `irfs` | `irfInterface::Irfs` (`aeff()->value/upperLimit`, `psf()->appDir`, `edisp()->appEnergy`, `efficiencyFactor`, `irfID`) in the library. `irfLoader::Loader` (`go`, `respIds`), `IrfsFactory` and `irfUtil::EventTypeMapper` in `gtobssim`. CMake links `irfInterface` to the library and `irfLoader`/`irfUtil` to the tool. |
| Depends on | `dataSubselector` | `Cuts` (energy/cone/IRF/event-type cuts, `accept(map)`, DSS keywords, GTI extension), `BitMaskCut`, `Gti`. See `src/dataSubselector/doc/architecture/`. |
| Depends on | `fitsGen` | `PRIVATE`. `Ft1File` in `EventContainer`, `Ft2File` in `ScDataContainer`. See `src/fitsGen/doc/architecture/`. |
| Depends on | `astro` | `GPS` (orbit, attitude, zenith, `earthpos().insideSAA()`), `PointingTransform`, `SkyDir`, `JulianDate`, `EarthCoordinate::geolat()` for FT2 `GEOMAG_LAT`. See `src/astro/doc/architecture/`. |
| Depends on | `tip` | Reads FT2 in `LatSc` and `get_tstart`. Header keywords on the outputs. |
| Depends on | `st_app`, `hoops`, `st_stream` | Tool framework and parameters (`AppParGroup`, which `ContainerBase` keeps so it can write `SIM*` keywords), `hoops::Hexception`, `StreamFormatter`. See `src/st_app/doc/architecture/`. |
| Depends on | `st_facilities`, `facilities` | `FitsUtil::writeChecksums`, `Util::fileExists`/`readLines`/`isXmlFile`/`expectedException`, `Environment::xmlPath`, `facilities::Util::expandEnvVar`, `Timestamp`. Neither is in `target_link_libraries`; both arrive transitively (through `dataSubselector`, `irfs` and `st_app`). |
| Depends on | CLHEP | `CLHEP::RandomS` (`RandFlat`, `HepRandom` seeding) and `CLHEP::GeometryS` (`Hep3Vector`, `HepRotation`). |
| Used by | — | No package includes `observationSim/` headers or imports it. The **tool** is run by `pyBurstAnalysisGUI` (`GtBurst/dataHandling.py`, `self.gtobssim = GtApp('gtobssim')`) and by `sane/python/tests/obsSim_tests.py`. The `obsSim` wrapper in `sane/python/gt_apps.py` is commented out. `modelEditor` reads and writes source XML in the same format. |

## Key abstractions

```
ObsSim (st_app::StApp, src/obsSim/obsSim.cxx)
  ├── Simulator ───────────── FluxMgr, CompositeSource (flux)
  │     makeEvents loop
  ├── EventContainer : ContainerBase ── Cuts (dataSubselector), Ft1File (fitsGen)
  ├── ScDataContainer : ContainerBase ─ Ft2File (fitsGen)
  └── Spacecraft (abstract)
        ├── LatSc     (src/, private) ── astro::GPS singleton; optional FT2 livetime
        └── EgretSc   (src/, private; compiled, unused)
Event, ScData: plain value classes buffered by the containers
```

| Type | Header | Role |
|------|--------|------|
| `Simulator` | `observationSim/Simulator.h` | Owns a `FluxMgr` and a `CompositeSource` with the named sources (plus the `obsSim_timetick30s` clock when no FT2 is used). `init` sets `GPS` time, the rocking or pointing history, and `EventSource::totalArea`. `generateEvents(double simTime, …)` runs for a span of time. `generateEvents(long n, …)` runs until *n* events are accepted. Both call `makeEvents`. |
| `EventContainer` | `observationSim/EventContainer.h` | `addEvent` does the IRF processing: acceptance, IRF choice, PSF/Edisp sampling and cuts. It buffers `Event`s and writes an FT1 file every `maxrows` events and in its destructor. It also keeps `SourceSummary` (id, incident count, accepted count) per source name. |
| `ScDataContainer` | `observationSim/ScDataContainer.h` | Buffers one `ScData` row per clock tick and writes FT2 every `maxrows` rows and in its destructor. It writes nothing if `writeData` is false (FT2 supplied). |
| `ContainerBase` | `observationSim/ContainerBase.h` | Output naming (`<root>_NNNN.fits`, 4 digits, then unpadded), `CREATOR`, and the `SIM*` parameter keywords. |
| `Spacecraft` | `observationSim/Spacecraft.h` | Abstract interface for attitude (`zAxis`, `xAxis`, `InstrumentToCelestial`), position, zenith, `inSaa` and `livetimeFrac`. |
| `LatSc` | `src/LatSc.h` (not installed) | All geometry comes from `astro::GPS::instance()`. `LatSc(ft2file)` also reads `START`/`STOP`/`LIVETIME` from `SC_DATA`, so `livetimeFrac(t)` is the FT2 row's `LIVETIME/(STOP-START)`. Without an FT2 file it is the constant `ltfrac`. |
| `Event`, `ScData` | `observationSim/Event.h`, `ScData.h` | Value types. `Event::theta/phi/zenAngle` are computed from the **apparent** direction and the axes stored when the event was made. |

## Data flow and control flow

### `gtobssim` run

```
ObsSim::run
  promptForParameters     infile, srclist, scfile, evroot, simtime, tstart, [use_ac → ra, dec, radius], irfs, seed
  checkOutputFiles        clobber=no and <evroot>_events_0000.fits or _scData_0000.fits exists → std::exit(1)
  setRandomSeed           CLHEP::HepRandom(seed) → seeds the static CLHEP engine
  createFactories         celestialSources::SpectrumFactoryLoader
  setXmlFiles             time_source.xml + (infile: an .xml file, or a list of files; default xmlFiles.dat)
  readSrcNames            srclist lines ('#' comments)
  createResponseFuncs     irfLoader::Loader::go(); respIds()[irfs (+ " (evtype)")] → vector<Irfs*>
  createSimulator         area = Σ aeff upperLimit / 1e4 m² (×1.5 if efficiency factor; `area` par if irfs=none)
                          tstart (INDEF → START of first FT2 row, or 0); + startdate offset
                          new Simulator(...); setIdOffset(offset); [no FT2: setRocking(3, rockangle)]
  generateData
    Cuts: ENERGY [emin, emax]; setIrfs(irfs) (EVENT_CLASS mask, CONVERSION_TYPE for ::FRONT/::BACK);
          remove IRF_VERSION; EVENT_TYPE mask for evtype; [cone]
    EventContainer(evroot_events, …, cuts, maxrows, tstart, tstop, edisp, pars)
    ScDataContainer(evroot_scData, …, maxrows, writeScData = no usable scfile)
    LatSc() or LatSc(scfile); setLivetimeFrac(ltfrac)
    Simulator::generateEvents(simtime | nevents)
    [orbit mode: add one more FT2 row at last tick + 30 s]
    saveEventIds → <evroot>_srcIds.txt   (id  name  incident  accepted)
  ~ScDataContainer, ~EventContainer       write the final FT2 / FT1 files
```

### The simulation loop (`Simulator::makeEvents`)

```
while !done():                                  // elapsed > maxtime, or elapsed ≥ simtime, or accepted ≥ n
  if no pending event:
     e = composite->event(absTime)             // next photon or clock tick from any source
     !e->enabled() → stop;  PointingHistory::TimeRangeError → stop ("Caught TimeRangeError")
     interval = composite->interval(absTime)
  if counting events, or elapsed + interval < simtime:
     absTime += interval;  fluxMgr->pass(interval)
     TimeTick and no FT2  → scData.addScData(e, spacecraft)        // one FT2 row per 30 s
     else                 → events.addEvent(e, irfs, spacecraft)   // counts toward n if accepted
  else:                                         // next photon falls after the window
     advance absTime to the end of the window and stop
```

### Per-photon processing (`EventContainer::addEvent`)

1. Get the true direction: `sourceDir = InstrumentToCelestial(t) · (−launchDir)`.
   The spacecraft axes come from `GPS` at time `t`.
2. Increment the incident count for the source.
3. **`irfs=none`**: store the photon with its true energy and direction and
   stop here. No livetime, SAA, area or cuts are applied (see gotchas).
4. Otherwise, accept the photon only if all of these hold:
   - `RandFlat < m_prob` (always 1 in `gtobssim`)
   - `RandFlat < livetimeFrac(t)`
   - `!inSaa(t)`
   - `drawRespPtr` succeeds. It computes `Aeff_i(E, dir, t) × efficiency`
     for each IRF, builds a cumulative sum, and draws
     `ξ = RandFlat × totalArea(cm²)`. The photon is rejected if
     `ξ ≥ ΣAeff`; otherwise the IRF whose bin contains ξ is chosen.
5. Sample `appDir = psf->appDir(...)`. If `edisp=yes` and the source allows
   it, sample `appEnergy = edisp->appEnergy(...)`.
6. Check `cuts->accept({ENERGY, RA, DEC, CONVERSION_TYPE = irfID % 2})`.
   Cuts on columns missing from that map (EVENT_CLASS, EVENT_TYPE) pass
   automatically.
7. Drop the photon if it is less than 26 µs after the previous buffered
   event.
8. Store the event:
   - `EVENT_CLASS` = the mask of the `EVENT_CLASS` bit-mask cut implied by
     `irfs` (the same for every event)
   - `EVENT_TYPE` = `1 << irfID`
   - `CONVERSION_TYPE` = `irfID == 1 ? 1 : 0`
   - `MC_SRC_ID` = the flux source id
   - `MCENERGY` = the true energy

### Writing FT1 (`EventContainer::writeEvents`)

1. Choose the template. If `PASS_VER` starts with `P7` or is `NONE`, use
   `ft1_p7.tpl` (integer `EVENT_CLASS`). Otherwise use `ft1.tpl` (`32X`).
2. Add the `MC_SRC_ID` (`1J`) and `MCENERGY` (`1E`) columns.
3. Fill `TIME`, `ENERGY`, `RA`, `DEC`, `L`, `B`, `THETA`, `PHI`,
   `ZENITH_ANGLE`, `EARTH_AZIMUTH_ANGLE`, `EVENT_CLASS`, `EVENT_TYPE` and
   `CONVERSION_TYPE`.
4. Write `PASS_VER`, one GTI `[start, stop]`, a GTI cut plus the DSS
   keywords, `FILENAME`, `VERSION` and `CREATOR`, and the `SIM*` keywords
   (`SIMINFIL`, `SIMSRCLI`, `SIMSCFIL`, `SIMOFFSE`, `SIMEDISP`, `SIMIRFS`,
   `SIMSEED`, and `SIMAREA` if `irfs=none`).
5. Write checksums.
6. The next file starts at this file's stop time.

### Writing FT2 (`ScDataContainer::writeScData`, orbit mode only)

- Each 30 s tick becomes one row, `START = tick`, `STOP = next tick`. The
  last row's `STOP` is extrapolated by one interval.
- `LIVETIME = ltfrac × (STOP − START)`, or 0 when `IN_SAA`.
- Position columns: `SC_POSITION` (m), `LAT_GEO`, `LON_GEO`, `RA/DEC_ZENITH`.
- Attitude columns: `RA/DEC_SCZ`, `RA/DEC_SCX`, `QSJ_1..4`.
- `GEOMAG_LAT` comes from `EarthCoordinate::geolat()` at the row midpoint.
- `DATA_QUAL = 1`, `LAT_CONFIG = 1`.

### Time handling

- Every time inside the simulation is MET in seconds. `GPS`, `flux` and the
  FT1 `TIME` column share one clock, `Simulator::m_absTime`.
- `tstart`:
  - If given, it is used as is.
  - If `INDEF` and an FT2 file is given, the `START` of its first row is
    used. `TSTART` is deliberately not used, because "TSTART from the
    Fermi astroserver is unreliable".
  - If `INDEF` and there is no FT2 file, it is 0, i.e. 2001-01-01.
- `startdate` (hidden, default `2001-01-01 00:00:00`) defines
  `offset = startdate − astro::JulianDate::missionStart()` in seconds.
  `offset` is added to `tstart`, passed to `Spectrum::setStartTime`
  (time-dependent `celestialSources` models subtract it to get their own
  local time), and passed to `GPS::setPointingHistoryFile` as the FT2 time
  offset. With the default, `offset = 0`.
- `simtime` is elapsed simulated seconds. With `nevents=yes` it is the
  number of **accepted** events instead. `maxtime` (hidden, 3.155e8 s,
  about 10 years) caps the elapsed time in either mode.
- With an FT2 file the run stops early, without an error, once a source
  asks for a time past the end of the file (`TimeRangeError`).

### Random numbers and reproducibility

- `setRandomSeed` seeds CLHEP's **static** engine (`HepRandom::setTheSeed`;
  the default engine in the bundled CLHEP 2.4.7.1 is `MixMaxRng`).
- observationSim draws with `RandFlat::shoot()`. `flux`, `celestialSources`
  (non-ROOT parts) and the `irfs` PSF/Edisp samplers also use the CLHEP
  static engine, so a single seed controls the whole run. ROOT-based sources
  (GRB, GRBobs, the ROOT `Pulsar`) use `TRandom` and are built only with
  `FERMI_BUILD_ROOT=ON`.
- The run is single-threaded and draws in a fixed order. The same seed,
  inputs, IRFs (CALDB version), source list order and build should produce
  identical events (inferred from the code; not checked at runtime). The
  `DATE` keyword and checksums still differ between runs.
- Changes that alter the random sequence, and so the events:
  - the order of `srclist`
  - `edisp`, `ltfrac`, `emin`/`emax`/cone (they change which draws are
    consumed)
  - the CLHEP version (CLHEP 2.4 changed the default engine)
- `SIMSEED` in the FT1/FT2 headers records the seed.

## Public interfaces

### Application

| Tool | Par file | Notes |
|------|----------|-------|
| `gtobssim` | `pfiles/gtobssim.par` | Prompted: `infile`, `srclist`, `scfile`, `evroot`, `simtime`, `tstart`, `use_ac` (and then `ra`, `dec`, `radius`), `irfs` (default `P7SOURCE_V6`), `seed` (default 293049). Hidden: `sctable`, `evtable`, `ltfrac` (0.9), `nevents`, `maxtime`, `startdate`, `offset` (source-ID offset), `rockangle` (INDEF → 35°), `emin`/`emax` (1 MeV / 1 TeV), `edisp` (yes), `evtype` (`none`/`PSF`/`EDISP`), `area` (m², only for `irfs=none`), `maxrows` (1e6). |

Startup, exit status and the standard hidden parameters follow `st_app`
(see `src/st_app/doc/architecture/`), except that `checkOutputFiles` and
`Simulator::init` (no valid source) call `std::exit(1)` directly. User help
is in `fermitools-fhelp`.

### Inputs

- **`infile`**: either an XML source library, or a text file listing XML
  files (one per line, `#` comments, `$(VAR)` expanded). `none` means
  `xml/xmlFiles.dat`, which lists
  - `$(OBSERVATIONSIMXMLPATH)/3EG_catalog_20-1e6MeV.xml`
  - `obsSim_source_library.xml`
  - `$(GRBXMLPATH)/GRB_user_library.xml`

  The GRB file is missing in non-ROOT builds and is skipped with a "File
  not found" message. `time_source.xml` (the `obsSim_timetick30s` clock
  and a `null_source`) is always loaded first.
- **`srclist`**: source names, one per line. A name that is not found is
  reported and skipped. If none are found the tool exits.
- **`scfile`**: FT2 file (`SC_DATA`), or `none` for the simulated orbit.

### Outputs

- `<evroot>_events_NNNN.fits`: FT1 with `EVENTS` + `GTI`, at most `maxrows`
  events per file.
- `<evroot>_scData_NNNN.fits`: FT2, only when `scfile` is `none` or missing.
- `<evroot>_srcIds.txt`: `MC_SRC_ID  name  incident  accepted` per source.

### C++ library

Installed headers: `Simulator.h`, `EventContainer.h`, `ScDataContainer.h`,
`ContainerBase.h`, `Spacecraft.h`, `Event.h`, `ScData.h`. Nothing outside
this package uses them. `LatSc.h` and `EgretSc.h` are private (`src/`), so
an external user would have to write its own `Spacecraft`.

### Python

None built. `python/observationSim.i` (SWIG) has no CMake target.
`python/latSim.py` and `python/test.py` are 2003-era scripts that import
`numarray` and an old `latResponse` module. They are not installed.

### Environment variables

| Variable | Read in | Effect |
|----------|---------|--------|
| `DISABLE_SAA` | `LatSc::inSaa` | If set (any value), SAA passages are ignored: no photons dropped, and `IN_SAA = F` in the FT2. |
| `IGNORE_IGRF_BOUNDARY` | `astro` (`IGRField`) | Lets FT2 writing work after the IGRF limit (see gotchas). |
| `ZENITH_FRAME_FIX` | `astro::GPS`, `flux` | Changes the frame of `ZENITH` direction sources. See `src/astro/doc/architecture/`. |
| `$(OBSERVATIONSIMXMLPATH)`, `$(GRBXMLPATH)`, … | XML file lists | Set by `setupEnvironment()` from `FERMI_INST_DIR`. See "Runtime environment" in ScienceTools' `docs/architecture/README.md`. |

## Build and test

- **CMake targets:**
  - `observationSim`, a **STATIC** library (PIC)
  - `gtobssim`, which also links `irfUtil`, `irfLoader` and `celestialSources`
  - `test_observationSim`
- **Install:**
  - headers to `include/observationSim`
  - `pfiles/` to `syspfiles`
  - `xml/` to `${FERMI_INSTALL_XMLDIR}/observationSim`. This is where
    `Environment::xmlPath("observationSim")` and `OBSERVATIONSIMXMLPATH`
    point. It includes the 3EG catalogue XML, `obsSim_source_library.xml`,
    `time_source.xml` and the converter scripts `3EG_to_xml.py` and
    `bpl_source.py`.
  - `data/` to `${FERMI_INSTALL_REFDATADIR}/observationSim`: the
    `TransientTemplate` light curves for 3C 279, PKS 1622−297, the Crab
    and Geminga, used by `obsSim_source_library.xml`, plus sample
    `source_names.dat`/`xml_files.dat`.
- **`test_observationSim`** (`src/test/main.cxx`):
  - It is a plain `main()`, not an `st_app` tool, so it does not need
    `PFILES`. It does need `FERMI_INST_DIR` to find the XML files. It is
    **not registered with CTest**.
  - Usage: `test_observationSim [count] [-t] [-fb] [source …]`.
  - By default it simulates 1000 accepted events from `all_3EG_sources`
    with the `DC1A::Front`/`DC1A::Back` IRFs, starting at MET 0 with the
    simulated orbit.
  - It writes `test_events_0000.fits` and `test_scData_0000.fits` to the
    **current directory**, so run it from a scratch directory.
  - It makes no assertions. It passes if it does not throw. `-fb` is
    parsed but has no effect.
  - `TRAP_FPE` is on for this target (not on macOS).
- **ST-unit-test:** the default run of ScienceTools'
  `recipe/tests/ST-unit-test` (`unitTestsWithoutRoot`) does **not** run
  `test_observationSim`. Only the ROOT list (`--with-root`) has it, followed
  by `check_test_observationSim_output`, which `ftdiff`s
  `test_events_0000.fits` against
  `share/fermitools/data/observationSim/outref/` and only checks that
  `test_scData_0000.fits` exists. That reference data is not in this repo:
  it ships in the `fermitools-test-scripts-data` conda package, sourced
  from `outrefFiles/observationSim/` in `fermitools-data` (which also holds
  an unused `test_scData_0000.fits`). See "Testing" in ScienceTools'
  `docs/architecture/README.md`.
- **Regression coverage** of the full tool exists only in `sane`'s Python
  tests, which run outside CTest and outside `ST-unit-test`.
- **Legacy:** `SConscript`, `observationSimLib.py` and
  `azure-pipelines.yml`. `doc/results.ps` is an old validation plot.

## Invariants and gotchas

**Expiring data: IGRF**
- In orbit mode (no `scfile`) each FT2 row's `GEOMAG_LAT` calls
  `astro::EarthCoordinate::geolat()`. That throws for any MET past about
  **7.56e8 (mid-December 2024)**, because the IGRF-13 coefficients stop at
  2025 and `EarthCoordinate` approximates the year as `2001 + MET/3.15e7`
  (see `src/astro/doc/architecture/`). So orbit-mode simulations of recent
  or future dates fail when the FT2 is written:
  - If the buffer fills (`maxrows`), the exception propagates.
  - At the end of the run, the write happens in `~ScDataContainer`. A
    throw from a destructor calls `std::terminate`, and since
    `ScDataContainer` is destroyed before `EventContainer`, **the FT1 file
    would not be written either** (from reading the code; not checked at
    runtime).
  - Workarounds: set `IGNORE_IGRF_BOUNDARY=1` (uses the 2025 field), or
    supply an FT2 file (`scfile`). With an FT2 file no FT2 is written and
    `geolat()` is never called.

**Event class and type**
- Every simulated event gets the **same** `EVENT_CLASS`: the mask of the
  class implied by `irfs` (for example only the SOURCE bit for
  `P8R3_SOURCE_V3`). Real Pass 8 data also sets the bits of looser classes,
  so a `gtselect` on a different `evclass` (TRANSIENT, CLEAN, …) keeps none
  or all of the simulated events, unlike real data (inferred).
- `EVENT_TYPE` has the single bit `1 << irfID` of the IRF that was drawn.
  With `evtype=PSF` or `EDISP`, events carry only that bit and no
  FRONT/BACK bit.
- `CONVERSION_TYPE` is written as `irfID == 1 ? 1 : 0`, but the cut test
  uses `irfID % 2`. For front/back IRFs (ids 0 and 1) these agree. For
  PSF/EDISP types (ids 2–9) the column is always 0 (from reading the code).
- The default `irfs=P7SOURCE_V6` is a Pass 7 IRF, which gives the
  `ft1_p7.tpl` layout. Most users must set a Pass 8 IRF. Whether
  `P7SOURCE_V6` is still in the distributed CALDB was not checked.

**Time keywords and GTIs**
- The FT1 GTI is one interval `[tstart, tstop]`. It ignores SAA passages,
  FT2 gaps and an early stop at the end of the FT2 file. Exposure is still
  right, because it comes from FT2 `LIVETIME`, but the `GTI` and `TSTOP`
  can overstate coverage.
- With `nevents=yes`, the stop time passed to `EventContainer` equals
  `tstart`. Unless `tstart` is 0, the last FT1 file then gets
  `TSTOP = TSTART` and a zero-length GTI (from reading the code; not
  checked at runtime).
- With a non-default `startdate`, `writeEvents` adds `Spectrum::startTime()`
  (the offset) to a start time that already includes it, so
  `TSTART`/the GTI would be shifted by the offset relative to `TIME`
  (from reading the code; not checked at runtime).
- No FT1 file is written if no events are accepted (the destructor writes
  only a non-empty buffer).

**Acceptance and areas**
- Photons are generated over a sphere of cross-section `totalArea`
  (Σ `aeff()->upperLimit()`, ×1.5 when the IRFs have an efficiency
  factor) and thinned by Aeff/`totalArea`. If Aeff × efficiency ever
  exceeded `totalArea`, the thinning would saturate and under-count
  (inferred; the ×1.5 is the guard).
- `totalArea` is set through the static `EventSource::totalArea` on the
  source named `default`. If the user's `infile` does not define
  `default`, the call goes through a null pointer to a static method.
  This works in practice, but it is undefined behaviour.
- The acceptance cone (`use_ac`) and `emin`/`emax` are applied **after**
  PSF and energy-dispersion sampling. Every source in `srclist` is still
  simulated over the whole sky, so a cone does not make the run faster.
- **`irfs=none`** is pass-through. It skips livetime, SAA, the
  `emin`/`emax`/cone cuts and the deadtime check, and writes true energy
  and direction with `EVENT_CLASS = EVENT_TYPE = 0`. The DSS keywords are
  still written as though the cuts had been applied.
- With an FT2 file, `ltfrac` is ignored and the livetime fraction comes
  from each FT2 row. Photons after the last row's `START` get livetime 0
  and are all rejected.
- The 26 µs spacing check compares only with the previous **buffered**
  event, so it is skipped for the first event of each output file.

**Spacecraft input**
- `LatSc(scfile)` always reads the `SC_DATA` extension. `sctable` is used
  only to look up `tstart`.
- A misspelled `scfile` silently falls back to the simulated orbit
  ("Pointing history file not found … Using default rocking strategy"),
  and an FT2 file is written.

**FT2 output (orbit mode)**
- Columns left unset (expected to be zero/default; inferred):
  - `RAD_GEO`, `B_MCILWAIN`, `L_MCILWAIN`, `LAMBDA`
  - `RA_NPOLE`, `DEC_NPOLE`, `ROCK_ANGLE`, `LAT_MODE`
  - `RA_SUN`, `DEC_SUN`, `SC_VELOCITY`

  Filters on `ROCK_ANGLE` or `LAT_MODE` therefore behave differently than
  on real FT2.
- The last row's `STOP` is extrapolated from the previous row, which needs
  at least two rows in the buffer. If a `maxrows` flush leaves exactly one
  row for the final file, `m_scData[npts-2]` is out of range (from reading
  the code).
- FT1 columns that are never filled: `EVENT_ID`, `RUN_ID`,
  `RECON_VERSION`, `CALIB_VERSION`, `LIVETIME`, `DIFRSP0..4`.

**Dead code**
- `ContainerBase::writeDateKeywords`/`currentTime` (mission start
  2006-12-31) are never called. The date keywords come from `fitsGen`.
- `EgretSc` is compiled into the library but nothing uses it.
- `EventContainer::m_eventType` is set only when there is no `EVENT_CLASS`
  cut, and it is never read.

**Unmerged branches** (as of `last_reviewed_commit`)
- `origin/addition_creator_fits` (2025-08) puts the ScienceTools version in
  `CREATOR` and raises the default `chatter` in `gtobssim.par` from 2 to 3.
- `origin/f26-cpp17-compliance` (2026-10) contains C++17 cleanups across
  `src/`.
- `origin/Issue17` (2022) is a one-line CMake change.

## Extension points

- **New source type:** implement it in `celestialSources` (or `flux`) and
  register it in `SpectrumFactoryLoader`. Then reference it from an XML
  source library. No change is needed here.
- **New output column:** set it in `EventContainer::writeEvents` (add it
  with `appendField` or to the `fitsGen` templates), and carry any new
  per-event value through `Event`.
- **New spacecraft model:** subclass `Spacecraft` and construct it in
  `ObsSim::generateData`. `LatSc` delegates everything to `astro::GPS`, so
  new orbit or rocking modes usually belong in `astro`/`flux`
  (`setRockType`).
- **New parameter recorded in headers:** add it to the par file and to
  `ContainerBase::writeParFileParams`.

Before merging, run `test_observationSim` from a scratch directory and a
short `gtobssim` in both orbit and FT2 modes. Check the FT1 file with
`gtvcut` (DSS keywords) and `gtselect`.

## Further reading

- `src/flux/` and `src/celestialSources/`: the source XML format and the
  source classes. (Their architecture docs are not written yet.)
- `src/astro/doc/architecture/`: `GPS`, rocking, pointing history, SAA and
  the IGRF expiry.
- `src/irfs/`: `irfInterface::Irfs`, `IPsf::appDir` and
  `IEdisp::appEnergy` sampling.
- `src/dataSubselector/doc/architecture/`: `Cuts::setIrfs` and the DSS
  keywords written here.
- `src/fitsGen/doc/architecture/`: the FT1/FT2 writers and templates.
- FSSC `gtobssim` documentation and the "observation simulation" analysis
  thread for user-level usage and source XML examples.
