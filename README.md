# Relational Database Management System (RDBMS) in C++

## Overview

This project is a simplified Relational Database Management System (DBMS) built using C++. It is a demonstration of core database management functionalities including storage, indexing, and basic query processing.

## Architecture

### Storage Manager:
* Multiple file database
* Slotted Pages design with 4kb size
* Storing data in Tuples
* Simple hash index Concept

### Query life cycle:
* Query parser for SQL commands
* Query analyzer to check query validity, semantics and generate initial plan
* Query optimizer for the best execution plan
* Query Execution engine

### Memory:
* Buffer pool in memory to load pages and make operations into 
