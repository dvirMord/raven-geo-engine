# Ground height (DSM) in raven-geo-engine

This page explains how the engine knows **how high the ground is** at any point, from zero.

---

## 1. Why we need it

The engine shoots a ray from the drone's camera toward the ground. To know **where the ray hits**, it has to know where the ground is.

Before this, the engine assumed the ground was **flat** (task #30). Real ground has hills and buildings, so we need a map of real heights. That map is the **DSM**.

---

## 2. What a DSM is

A **DSM** (Digital Surface Model) is a big table of heights.

- Each box in the table is called a **cell**.
- Each cell covers a small square of the Earth (about 30 × 30 m in our data).
- Each cell holds **one number**: the height in meters at that square.

```text
            west ─────────────────────────► east
 north   [ 1890 ] [ 1893 ] [ 1897 ] [ 1901 ] ...
   │     [ 1888 ] [ 1891 ] [ 1895 ] [ 1899 ] ...
   │     [ 1885 ] [ 1889 ] [ 1892 ] [ 1896 ] ...
   ▼     ...
 south
```

**"Surface"** means it measures what you'd see from the sky: ground **plus** buildings and trees.
That's what we want, because the camera sees roofs, not the ground under them.
(A **DTM**, Digital Terrain Model, is bare ground only. We don't use it.)

---

## 3. Our data

| | |
| --- | --- |
| Source | **Copernicus GLO-30** (free, worldwide) |
| Cell size | 1 arc-second ≈ **30 m** |
| Height measured from | **sea level** (EGM2008), the same as the drone's altitude in the telemetry |
| Files | one file per 1° × 1° square, called a **tile** |

The tiles live in `TheProject\DEM\copernicus\`:

| File | Covers | Videos |
| --- | --- | --- |
| `N41W105_Cheyenne_truck.tif` | lat 41–42, lon −105 to −104 | Cheyenne, Cheyenne_Handoff, CheyenneVAhospital, truck |
| `N27W082_Florida_Esri_multiplexer.tif` | lat 27–28, lon −82 to −81 | Esri_multiplexer_0, Esri_multiplexer_1 |
| `N35W080_NorthCarolina_klv_metadata_test_sync.tif` | lat 35–36, lon −80 to −79 | klv_metadata_test_sync |

The name starts with the tile's south-west corner: `N41W105` = latitude 41, longitude −105.

Download source: `https://copernicus-dem-30m.s3.amazonaws.com/` (original names are in the folder's `README.md`).

---

## 4. What's inside a `.tif` file

A `.tif` is an image file. Normally each pixel is a color. Here, **each pixel is a height**.

Think of the file as a **book**:

```text
┌───────────────────────────────────────────────┐
│ Table of contents (the "tags")                │
│   size:        3600 × 3600 cells              │
│   value type:  32-bit decimal number (float)  │
│   compression: deflate (like ZIP)             │
│   corner:      lat 42, lon −105   ← tag 33922 │
│   cell size:   0.00027778°        ← tag 33550 │
├───────────────────────────────────────────────┤
│ Chapters: 16 blocks of 1024 × 1024 heights,   │
│ each one zipped separately                    │
│  ┌────┬────┬────┬────┐                        │
│  │ 0  │ 1  │ 2  │ 3  │                        │
│  ├────┼────┼────┼────┤                        │
│  │ 4  │ 5  │ 6  │ 7  │  3600 / 1024 = 3.5,    │
│  ├────┼────┼────┼────┤  so the last row and   │
│  │ 8  │ 9  │ 10 │ 11 │  column of blocks are  │
│  ├────┼────┼────┼────┤  only half full        │
│  │ 12 │ 13 │ 14 │ 15 │                        │
│  └────┴────┴────┴────┘                        │
└───────────────────────────────────────────────┘
```

Because the chapters are zipped, you can't just jump to a cell. You have to unzip its block first.
We use the **libtiff** library for that: it reads the table of contents and unzips the blocks.

**GeoTIFF** is a normal TIFF with two extra tags that say **where on Earth** the table is:

| Tag | Name in the standard | What it holds | We use |
| --- | --- | --- | --- |
| 33550 | ModelPixelScale | `[lon size, lat size, 0]` = size of one cell in degrees | `[0]`, `[1]` |
| 33922 | ModelTiepoint | `[col, row, 0, lon, lat, 0]` = "cell [0][0] is at this lon/lat" | `[3]`, `[4]` |

---

## 5. From a point to a height

Given a point (latitude, longitude), we find its cell like this:

```text
row = (corner latitude − latitude)    / cell size in latitude
col = (longitude − corner longitude)  / cell size in longitude
```

Then drop the decimals, and read the height.

**Why the subtraction is reversed between the two lines:**
- Rows count **south** from the corner, and latitude gets **smaller** going south → corner minus point.
- Columns count **east** from the corner, and longitude gets **bigger** going east → point minus corner.

**Example** (Cheyenne tile, corner 42 / −105, cell 0.00027778°), point 41.12784, −104.78987:

```text
row = (42 − 41.12784)    / 0.00027778 = 3139.7  →  3139
col = (−104.78987 + 105) / 0.00027778 =  756.5  →   756
height = table[3139][756] = 1838.78 m
```

**If the point isn't on this tile**, `row` or `col` comes out negative or too big. Then we try the next tile.
If no tile has it, the answer is **"no height"** (`nullopt`).

**How the table is stored in memory:** as one long line, row after row. Cell `[row][col]` is at position:

```text
row × number of columns + col
```

---

## 6. How the engine uses it

```text
engine starts
  └─ GeoTiffHeightProvider loads all 3 tiles into memory (once, ~1 s each, ~52 MB each)
       ├─ a tile that fails to load → warning in the log, skipped, the engine keeps going
       └─ "Loaded 3 of 3 terrain tiles."

every telemetry frame
  └─ getHeightMeters(lat, lon) → height in meters, or nullopt
```

The code:

| File | What it is |
| --- | --- |
| `Terrain/IHeightProvider.hpp` | The interface: one function, `getHeightMeters(latitude, longitude)` → height or `nullopt` |
| `Terrain/GeoTiffHeightProvider.hpp/.cpp` | The implementation that reads the `.tif` tiles |
| `Models/HeightTileHeader.hpp` | One tile in memory: corner, cell size, rows, columns, all heights |
| `Common/Constants.hpp` → `GeoTiff` | Tag numbers, array positions, and the tile paths (`TILE_PATHS`) |
| `main.cpp` | Creates the provider at startup and passes it to `GeoCalculator` |

**Why an interface:** the rest of the engine only asks "how high is the ground here?" and doesn't care where the answer comes from.
Later we can add another source (flat ground for testing, or a sharper lidar map) without changing the ray code.

**Why load everything at startup:** the ray code will ask for heights dozens of times per frame. Reading from memory takes nanoseconds; reading and unzipping from disk every time would be far too slow.

---

## 7. Setup on a new machine

1. Install libtiff with deflate support:
   ```text
   vcpkg install tiff[zip]:x64-windows
   ```
2. Download the 3 tiles (section 3) into a folder.
3. Point `Constants::GeoTiff::TILE_PATHS` in `Common/Constants.hpp` at them.
4. Build and run. The log should show `Loaded 3 of 3 terrain tiles.`

---

## 8. How we checked it works

Every frame, the engine logs the DSM height at the **frame center** next to the height the drone's own system reported for the same point (`FrameCenterElevation`):

```text
Ground height at frame center: telemetry 1914.88 m, DSM 1913.65 m, diff +1.23 m
```

We compare the **same point**, so the difference only says how much the two height maps agree.

Results:

| Flight | Frames | Average diff | Spread (std) | Range |
| --- | --- | --- | --- | --- |
| truck | 711 | +0.29 m | 1.44 m | −4.6 to +4.3 m |
| Esri_multiplexer_0 | 104 | −2.50 m | 3.17 m | −7.8 to +4.1 m |

A few meters of agreement means the tiles are read correctly. A wrong corner, a swapped row/column, or the wrong tile would show tens or hundreds of meters.

Note: `FrameCenterElevation` is **not** the ground truth. The drone computed it with its own height map, so this check shows agreement, not exact correctness.

---

## 9. Limits to know about

- **30 m cells blur small things.** A cell holds one value for its whole square, roughly an average.
  A house 10 m wide next to a lawn becomes a cell a few meters high.
  Hills and large buildings survive; poles, cars, and small houses mostly don't.
- **One height per cell.** We take the cell the point falls in, with no blending between neighbor cells (that's task #33).
- **No live download.** Tiles must be on disk before the engine starts.
- **The tile search is a simple loop.** That's fine for 3 tiles. With hundreds, look tiles up by name (`N41W105`) instead.
- **Open question:** one early Cheyenne_Handoff frame showed a 54 m difference. That flight hasn't been checked in full yet.

If 30 m turns out not to be sharp enough, the free option is to build a 1–2 m DSM from the USGS 3DEP lidar of Laramie County (2021).
It covers the Cheyenne flights, but it is 9 years newer than the video.

---

## 10. Glossary

| Word | Meaning |
| --- | --- |
| DSM | Height map of the surface, including buildings and trees |
| DTM | Height map of bare ground only |
| Cell | One box of the table, one height |
| Tile | One `.tif` file covering 1° × 1° |
| Block | One of the 16 zipped chunks inside a tile (libtiff calls it a "tile" too, confusingly) |
| GeoTIFF | A TIFF image with tags saying where on Earth it is |
| Resolution | Cell size. Smaller cells = **higher** resolution |
| Sea level / EGM2008 | The "zero" heights are measured from |
| Frame center | The point at the middle of the video frame, reported in the telemetry |
