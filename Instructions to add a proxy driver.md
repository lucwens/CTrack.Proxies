# Instructions to add a proxy driver

## Naming convention

We take the next name convention 
Company{.DriverType}

this is applied to :
* Source folder `/CTrack V5.0/Proxies/Company{.DriverType}/`
* Project name when copied from the template
* Binary folder `/CTrack V5.0/Bin64/Proxies/Company{.DriverType}/`
* Executable name for proxy: `Company{.DriverType}.exe`

Where Company is the name of the producer of the hardware and DriverType is an optional addition, e.g.
* NDI.Certus
* NDI.Pro
* Leica.LMF
* Leica.emScon
* Creaform

## Folder structure

### Source
```
/CTrack V5.0/Proxies
    ├── Libraries                   # Shared library files
    |   ├── Driver                  # IDriver interface 
    |   ├── TCP                     # TCP/IP routines
    |   ├── Utility                 # Error, string and console routines
    |   └── XML                     # XML support routines
    └── Company{.DriverType}
```

### Binaries

```
/CTrack V5.0/Bin64/Proxies
    └── Company{.DriverType}  # Contains Company{.DriverType}.exe + all DLL's
```
By using a seperate folder for all related binaries we avoid conflicts that might arise when different types of DLL's with the same name are used, as for example OAPI.dll which is different for Certus and Pro.

# Create the Proxy program
## Copy from template

Use CopyWiz (D:\Dropbox\Install\VCTools\CopyWiz).
![alt text](image.png)

![alt text](image-1.png)

* Add the newly created folder to SVN.
* Add the newly created project to the CTrack solution under the folder Proxies.
* Build the new project and check there are no errors
* Commit this version.

## Update project to external driver

Check the requirements for the external library:
- Is CLI required `(Advanced/Common Language Runtime Support)`, if yes make sure that UNICODE is used
- What is the binary compatibilty of the library, check for
-- Unicode or MCBS `(Advanced/Character set)`
-- Static or dynamic `(C/C++/Code Generation/Runtime Library)`

## Copy binaries and includes
* Copy the includes into the root folder
* Copy the library into the root folder
* Copy the DLL's to `/CTrack V5.0/Bin64/Debug/Proxies/Company{.DriverType}/`, the postbuild routines will copy the DLL's automatically to the Release folder.
* Add newly copied files to SVN
* Commit

## Implement driver functions
### Hardware detection
The hardware detection should at a minimum set the next attributes:

In addition the next attributes should be set

### Configuration detection

### CheckInitialize / Shutdown
### Run

# Implement a driver in CTrack
## Inherit from CProxyDevice and 