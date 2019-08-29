# OpenCL

## Table of Content

* [OpenCL](#opencl)
  * [Table of Content](#table-of-content)
  * [What is OpenCL](#what-is-opencl)
  * [OpenCL Architecture](#opencl-architecture)
    * [Platform Model](#platform-model)
    * [Execution Model](#execution-model)
    * [Memory Model](#memory-model)
    * [Programming Model](#programming-model)
  * [Case Studies](#case-studies)
    * [Image Rotation](#image-rotation)
    * [Matrix Multiplication](#matrix-multiplication)
  * [Memory Issues](#memory-issues)
    * [Coalescing Memory Accesses](#coalescing-memory-accesses)
    * [Memory Back Conflicts](#memory-back-conflicts)
  * [Synchronization](#synchronization)
    * [Within a Work Group](#within-a-work-group)
    * [Among Commands in Command Queue(s)](#among-commands-in-command-queues)
  * [Profiling Using Events](#profiling-using-events)

## What is OpenCL

* programming framework for heterogeneous compute resources

## OpenCL Architecture

### Platform Model

* each vendor supplies a single platform per implementation
* installable client driver model
  * co-exist between platforms from different vecdors
  * current systems' device driver model will not allow different vendors' GPUs to run at the same time
* host connected to one or more OpenCL devices
* compute device -> compute unit (CU) -> processing element (PE)
* in cuda term: GPU -> streaming multiprocessor (SM) -> stream processor (SP)

### Execution Model

### Memory Model

### Programming Model

## Case Studies

### Image Rotation

### Matrix Multiplication

## Memory Issues

### Coalescing Memory Accesses

### Memory Back Conflicts

## Synchronization

### Within a Work Group

### Among Commands in Command Queue(s)

## Profiling Using Events
