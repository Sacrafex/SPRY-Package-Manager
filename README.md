# SPRY-Package-Manager
The official bundled and mirrored package manager for SacrOS

# SPRY Package Manager

SPRY Package Manager is a secure, distributed package manager for Debian-based Linux systems. It allows users to install applications directly from upstream sources or trusted mirrors, using a single bundled binary for easy installation.

## Features

- Single binary installation via curl.
- Download applications directly from official sources or trusted mirrors.
- SHA256 checksum and optional GPG verification for security across mirrors.
- Cross-check files across multiple mirrors to detect tampering.
- Metadata-driven: JSON or AppStream XML index with app info, licenses, and screenshots.
- CLI-first workflow with commands for search, info, and install.
- Automatic desktop integration via `.desktop` entries.

## Installation

Run the following command to bootstrap SPRY (SPRY is preinstalled on all SacrOS Systems):

```bash
curl -sSL https://raw.githubusercontent.com/Sacrafex/SPRY/main/install.sh | sh

```
## Hosting a SPRY Mirror

Running a SPRY mirror allows users to download packages faster and ensures high availability. Follow these steps to set up your mirror.

### Minimum Requirements

- **Operating System:** SacrOS Server or Desktop, version 1.0.0 (Genesis)  
- **Network:** Stable connection with average bandwidth of 500 Mb/s or higher  
- **Memory:** 4 GB RAM or more  
- **Storage:** 180 GB or more available disk space  

### Recommended Configuration

- **Operating System:** SacrOS Server or Desktop, version 1.0.0 (Genesis) or later  
- **Network:** High-speed connection with average bandwidth of 1 Gb/s or higher  
- **Memory:** 8 GB RAM or more  
- **Storage:** 500 GB or more available disk space  
- **Additional:** SSD storage for faster read/write operations and reduced latency  

### 1. Prepare Your Server

Ensure your server meets the minimum or recommended requirements listed above.

### 2. Install Required Software

Install basic tools to serve files and manage mirrors:

```bash
sudo spry install spry-host
```
