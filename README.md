# Relational Database Management System (RDBMS) in C++

A simplified relational database management system built from scratch in C++17. Implements persistent storage with slotted pages, multi-page tables, a buffer pool with clock-replacement caching, and a full query processing pipeline from parsing to execution.

---

## Features

| Component             | Details                                                              |
|-----------------------|----------------------------------------------------------------------|
| **Storage Engine**    | Binary `.HAD` files, 4 KB slotted pages, 50-byte fixed-size tuples  |
| **Multi-Page Tables** | Automatic page allocation when a page fills up; all pages are scanned for queries |
| **Buffer Pool**       | Clock (Second-Chance) page replacement with pin/unpin and dirty tracking |
| **Query Parser**      | Regex-based SQL parser for `CREATE`, `INSERT`, `SELECT`, `UPDATE`, `DELETE` |
| **Execution Engine**  | Interprets parsed queries; manages all page I/O through the buffer pool |
| **Interactive Shell** | REPL with formatted table output, execution plans, and buffer inspection |

---

## Project Structure

```
DBMS/
├── include/                     # Header files
│   ├── Buffer.hpp               # Buffer pool & frame definitions
│   ├── DataBase.hpp             # Database catalog manager
│   ├── ExecutionEngine.hpp      # Query execution engine
│   ├── page.hpp                 # Page (slotted page) layout
│   ├── parser.hpp               # SQL parser & query analyzer
│   ├── Table.hpp                # Table metadata & page directory
│   └── tuple.hpp                # Tuple serialization/deserialization
├── src/
│   ├── buffer/
│   │   └── Buffer.cpp           # Clock-replacement buffer pool implementation
│   ├── engine/
│   │   └── ExecutionEngine.cpp  # CREATE, INSERT, SELECT, UPDATE, DELETE logic
│   ├── parser/
│   │   └── parser.cpp           # SQL parsing, validation, execution plan generation
│   ├── storage/
│   │   ├── DataBase.cpp         # Database directory & schema persistence
│   │   ├── page.cpp             # Page serialization, tuple CRUD on pages
│   │   ├── Table.cpp            # Table page allocation & management
│   │   └── tuple.cpp            # Tuple attribute serialization
│   └── main.cpp                 # Interactive SQL shell (REPL) entry point
├── Makefile                     # Build system
└── README.md
```

### On-Disk Layout

```
<database_name>/                 # One folder per database
├── students.HAD                 # One file per table
├── courses.HAD                  # Contains schema header + all data pages
└── ...
```

Each `.HAD` file contains:
```
[ Schema Header ][ Page 1 (4096 B) ][ Page 2 (4096 B) ][ ... ][ Page N ]
```

---

## Build & Run

### Prerequisites

- **C++17** compiler (GCC 8+, Clang 7+, or Apple Clang)
- **Make**

### Build

```bash
make          # Compile all source files → ./dbms
make clean    # Remove object files and executable
```

### Run

```bash
./dbms <database_name>
```

This creates a `<database_name>/` directory. Each `CREATE TABLE` adds a `.HAD` file inside it.

---

## Supported SQL Commands

### CREATE TABLE
```sql
CREATE TABLE students (name VARCHAR, age INT, city VARCHAR)
```

### INSERT
```sql
INSERT INTO students (name, age, city) VALUES (Alice, 25, Paris)
```

### SELECT
```sql
SELECT * FROM students
SELECT * FROM students WHERE name = Alice
```

### UPDATE
```sql
UPDATE students SET city = Berlin WHERE name = Alice
UPDATE students SET age = 99
```

### DELETE
```sql
DELETE FROM students WHERE name = Bob
DELETE FROM students
```

### Inspect Buffer Pool
```sql
buffer
```

---

## Architecture — Deep Dive

### 1. On-Disk Storage (How Data Lives on Disk)

#### Database = Folder

When you run `./dbms school`, the system calls `fs::create_directory("school")` to create a folder. All table files live inside this folder. This keeps databases isolated — different database names produce different directories.

#### Table = Single Binary File

Each table is a single file named `<table>.HAD` inside the database folder (e.g. `school/students.HAD`).

**File layout:**

```
Bytes 0..N:      Schema Header (variable length)
Bytes P*4096..:  Page 1 (exactly 4096 bytes)
Bytes 2*4096..:  Page 2 (exactly 4096 bytes)
...
Bytes K*4096..:  Page K
```

**Schema header** (binary, network byte order):

| Field           | Size    | Description                         |
|-----------------|---------|-------------------------------------|
| `schema_size`   | 4 bytes | Number of columns                   |
| `page_count`    | 4 bytes | Number of data pages in this file   |
| For each column:|         |                                     |
| `key_len`       | 4 bytes | Length of column name                |
| `val_len`       | 4 bytes | Length of column type string         |
| `key`           | N bytes | Column name (e.g. "name")           |
| `value`         | M bytes | Column type (e.g. "VARCHAR")        |

The `page_count` field is updated in-place (seekp to byte 4) every time a new page is allocated, so the engine always knows how many pages exist when it opens the file later.

#### Page = 4096-Byte Slotted Block

Each page is a fixed 4096-byte block with this internal layout:

```
+-----------------------------+
| freespace (4 bytes)         |  ← How many free bytes remain
| ids_Range.first (4 bytes)   |  ← Starting ID for tuples on this page
| ids_Range.second (4 bytes)  |  ← Ending ID range for this page
+-----------------------------+
| Tuple 0 (50 bytes)          |
| Tuple 1 (50 bytes)          |
| ...                         |
| Tuple N (50 bytes)          |
| [unused space padded to 0]  |
+-----------------------------+
          Total: 4096 bytes
```

- **Header**: 12 bytes (3 × int32). Stored in network byte order (`htonl`/`ntohl`) for portability.
- **Data area**: `4096 - 12 = 4084` bytes. Each tuple is 50 bytes, so one page holds at most **81 tuples** (`⌊4084/50⌋`).
- **Free space tracking**: `freespace` starts at 4084 for an empty page. Each `insert_tuple` decrements it by 50. When `freespace < 50`, the page is full.

#### Tuple = 50-Byte Fixed Record

Each tuple packs its attributes sequentially into exactly 50 bytes:

```
For each attribute:
  [1 byte: key_length]
  [N bytes: key string]
  [1 byte: type enum (1=STRING, 2=INT)]
  [1 byte: value_length]
  [M bytes: value string]
```

Attributes are written in order until the 50-byte boundary. Remaining bytes are zero-padded. On deserialization, the parser reads key_length → key → type → value_length → value in a loop until it hits a zero key_length or runs out of space.

An auto-generated `id` attribute is appended to every tuple on insert, using `tuples.size() + ids_Range.first` to ensure globally unique IDs across pages.

---

### 2. Multi-Page Table Management (How Pages Grow)

Tables start with one empty page. When a page fills up (81 tuples), the engine automatically allocates a new page:

**Insert logic (ExecutionEngine::insert):**
```
1. Load table metadata from disk (includes page_count)
2. For each page_id from 1 to page_count:
   a. Fetch page through buffer pool
   b. If page has free space → insert tuple, mark dirty, flush, done
   c. If page is full → unpin, try next page
3. If ALL pages are full:
   a. Call Table::Create_page() → allocates page (page_count+1)
   b. Serialize empty page to disk (extends the .HAD file by 4096 bytes)
   c. Update page_count in the schema header on disk
   d. Fetch the new page through buffer pool
   e. Insert tuple into the new page, mark dirty, flush
```

**Select / Update / Delete logic:**
```
1. Load table metadata (page_count)
2. For each page_id from 1 to page_count:
   a. Fetch page through buffer pool
   b. Scan all tuples on the page (sequential scan)
   c. Collect matches (SELECT) or modify in-place (UPDATE/DELETE)
   d. If modified → mark dirty, flush to disk
   e. Unpin page
3. Return aggregated results
```

This means every query scans all pages — a full table scan. There are no indexes (yet).

---

### 3. Buffer Pool (How Pages Are Cached in Memory)

The buffer pool sits between the execution engine and the disk. It caches recently-used pages in a fixed-size array of **frames** (default: 10 frames).

#### Frame Structure

Each frame holds:
| Field          | Purpose                                                  |
|----------------|----------------------------------------------------------|
| `page`         | Pointer to the in-memory `Page` object                   |
| `pageId`       | Which page number this frame holds                       |
| `dbName`       | Database name (for the file path)                        |
| `tableName`    | Table name (for the file path)                           |
| `pinCount`     | How many operations are currently using this frame        |
| `dirtyBit`     | Whether the page has been modified since last disk write  |
| `referenceBit` | Used by the clock algorithm to decide eviction            |

#### Page Table

A hash map (`unordered_map`) maps composite keys (`"dbName/tableName:pageId"`) to frame indices for O(1) lookup.

#### Clock Replacement Algorithm (Second-Chance)

When a page is requested but not in the pool, the algorithm must evict a frame:

```
clockHand starts at 0, sweeps circularly through frames:

1. If frame is EMPTY → use it immediately
2. If frame is UNPINNED (pinCount == 0):
   a. If referenceBit == false → EVICT this frame
      - If dirty → write page to disk first
      - Remove from page table
      - Load new page into this frame
   b. If referenceBit == true → set it to false (give second chance)
      - Move clockHand forward
3. If frame is PINNED (pinCount > 0) → skip it
   - Move clockHand forward
4. After 2 full sweeps with no victim → all pages pinned, return error
```

This gives frequently-accessed pages a "second chance" before eviction, approximating LRU without the overhead of maintaining a linked list.

#### Pin/Unpin Protocol

The execution engine follows a strict protocol:
```
Page* p = bufferPool.fetchPage(pageId, db, table);  // pinCount++
// ... read or modify p ...
bufferPool.markDirty(pageId, db, table);              // if modified
bufferPool.flushPage(pageId, db, table);              // write to disk
bufferPool.unpinPage(pageId, db, table);              // pinCount--
```

Pages with `pinCount > 0` are never evicted. Forgetting to unpin causes the pool to fill up and reject new page requests.

---

### 4. Query Processing Pipeline (From SQL Text to Results)

```
User types: "SELECT * FROM students WHERE name = Alice"
                              │
                              ▼
                    ┌──────────────────┐
                    │   SQL Parser     │
                    │  (SyntaxValidator)│
                    └────────┬─────────┘
                             │
         Regex matching against known SQL patterns.
         Extracts: type="SELECT", tableName="students",
                   columns=["*"], condition="name = Alice"
                             │
                             ▼  QueryInfo struct
                    ┌──────────────────────┐
                    │  Execution Plan Gen. │
                    └────────┬─────────────┘
                             │
         Produces a list of logical steps:
           1. Table Scan → students
           2. Filter → name = Alice
           3. Projection → *
                             │
                             ▼  vector<ExecutionStep>
                    ┌──────────────────────┐
                    │   Execution Engine   │
                    └────────┬─────────────┘
                             │
         For each page 1..page_count:
           fetchPage → get_tuple({"name","Alice"}) → collect results
           unpinPage
                             │
                             ▼
                    ┌──────────────────┐
                    │  Pretty Printer  │
                    └──────────────────┘
                             │
         Auto-detect column names from first tuple.
         Calculate column widths from data.
         Print aligned table with headers and row count.
```

#### Parser Details

The parser uses compiled `std::regex` patterns to match SQL syntax:

- **SELECT**: `SELECT (columns) FROM (table) [WHERE (condition)]`
- **INSERT**: `INSERT INTO (table) (columns) VALUES (values)`
- **UPDATE**: Splits into two patterns — one with WHERE, one without — to prevent the SET clause from consuming the WHERE keyword.
- **DELETE**: `DELETE FROM (table) [WHERE (condition)]`
- **CREATE TABLE**: `CREATE TABLE (name) (column definitions)`

The `splitAndTrim` helper tokenizes comma-separated lists and strips whitespace.

#### Condition Parsing

WHERE conditions like `name = Alice` are parsed by `parseCondition()`:
1. Find the `=` sign
2. Extract column name (left of `=`) and value (right of `=`)
3. Trim whitespace and quotes
4. Return as `{column, value}` pair

SET clauses like `city = Berlin, age = 30` are parsed by `parseSetClause()` — split by commas, then parse each `col = val` assignment.

---

### 5. Execution Engine Operations (How Each SQL Runs)

#### CREATE TABLE
1. Check table doesn't already exist (look for `<db>/<table>.HAD`)
2. Serialize the schema (column names + types) as the file header
3. Allocate one empty page (page 1) and append it to the file
4. Write `page_count = 1` into the schema header

#### INSERT
1. Open table metadata → get `page_count`
2. Scan pages 1..N through the buffer pool
3. Find first page with `freespace ≥ 50`
4. If none found → allocate new page, extend file, update `page_count`
5. Call `Page::insert_tuple`:
   - Create a `Tuple` object, add each column as an attribute
   - Auto-assign `id = tuples.size() + ids_Range.first`
   - Subtract 50 from `freespace`, push tuple into the in-memory vector
6. Mark page dirty → flush to disk → unpin

#### SELECT
1. Open table metadata → get `page_count`
2. For each page 1..N:
   - Fetch through buffer pool
   - Call `Page::get_tuple(condition)`:
     - If condition is `{" "," "}` (no WHERE) → return all tuples
     - Otherwise → filter tuples where `attribute[key] == value`
   - Append matches to results vector
   - Unpin page
3. Pretty-print results as an aligned table

#### UPDATE
1. Open table metadata → get `page_count`
2. For each page 1..N:
   - Fetch through buffer pool
   - Call `Page::update_tuple(condition, updates)`:
     - Find tuples matching the condition
     - For each match, call `Tuple::update_attribute` to overwrite values
   - If any tuple was modified → mark dirty, flush
   - Unpin page

#### DELETE
1. Open table metadata → get `page_count`
2. For each page 1..N:
   - Fetch through buffer pool
   - Call `Page::del_tuple(condition)`:
     - If condition is `{" "," "}` → clear all tuples, reset freespace
     - Otherwise → erase matching tuples, add 50 to freespace per deletion
   - Mark dirty → flush → unpin

---

## Example Session

```
$ ./dbms school
DBMS Interactive Shell. Type 'exit' to quit.

sql> CREATE TABLE students (name VARCHAR, age INT, gpa FLOAT)

Execution Plan:
  Create Table -> students (Creating new table)

Table 'students' created.

sql> INSERT INTO students (name, age, gpa) VALUES (Alice, 21, 3.8)

Execution Plan:
  Insert -> students (Inserting new row)

sql> INSERT INTO students (name, age, gpa) VALUES (Bob, 22, 3.5)

sql> SELECT * FROM students

Execution Plan:
  Table Scan -> students (Sequential scan of table)
  Projection -> * (Selecting columns)

 name   | age  | gpa  | id
--------+------+------+-----
 Alice  | 21   | 3.8  | 1
 Bob    | 22   | 3.5  | 2
(2 rows)

sql> UPDATE students SET gpa = 3.9 WHERE name = Alice

Execution Plan:
  Update -> students (Updating rows)
  Set -> gpa = 3.9 (Setting new values)
  Filter -> name = Alice (Applying WHERE clause)

Records updated in table 'students'.

sql> DELETE FROM students WHERE name = Bob

Execution Plan:
  Delete -> students (Deleting rows)
  Filter -> name = Bob (Applying WHERE clause)

Delete operation completed on table 'students'.

sql> buffer

+-------+---------------------+--------+-------+-------+--------+
| Frame |        Page         | Pinned | Dirty | Ref   | Tuples |
+-------+---------------------+--------+-------+-------+--------+
|     0 |          students:1 |      0 |     N |     Y |      1 |
|     1 |             (empty) |      - |     - |     - |      - |
+-------+---------------------+--------+-------+-------+--------+

sql> exit
Flushing buffer pool and shutting down...
```

### Multi-Page Example

```
sql> CREATE TABLE logs (msg VARCHAR)

-- Insert 85 rows (page 1 holds max 81 tuples)

sql> buffer

+-------+---------------------+--------+-------+-------+--------+
| Frame |        Page         | Pinned | Dirty | Ref   | Tuples |
+-------+---------------------+--------+-------+-------+--------+
|     0 |            logs:1   |      0 |     N |     Y |     81 |
|     1 |            logs:2   |      0 |     N |     Y |      4 |
+-------+---------------------+--------+-------+-------+--------+

-- Page 2 was automatically created when page 1 filled up.
-- SELECT * FROM logs returns all 85 rows across both pages.
```

---

## Limitations

- **Fixed tuple size**: Each tuple is 50 bytes. Attribute data that exceeds this is truncated.
- **No JOIN or subqueries**: Only single-table queries are supported.
- **No transactions or concurrency**: Single-user, single-threaded operation.
- **No indexes**: All queries perform sequential full-table scans across all pages.
- **Sequential page allocation**: New pages are always appended. Deleted tuples free space within a page but pages are never reclaimed.

---

## License

This project is for educational purposes.
