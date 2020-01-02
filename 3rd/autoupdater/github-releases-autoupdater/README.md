A C++ / Qt library for update checking and downloading the updates for the software distributed via GitHub releases.
Currently, the library is intended for downloading and launching a Windows installer (that will take care of the actual updating). If you want to extend it to support OS X / Linux / whatever - be my guest.

# Usage

1. Create an instance of the updater class. Specify your GitHub repository address and a string representation of the current version of the software (could be any format, just make sure it's consistent with your GitHub version tags):
  `_updater("https://github.com/VioletGiraffe/file-commander", "0.9.1")`
2. Specify the class that will receive update notification (via the `CAutoUpdaterGithub::UpdateStatusListener` interface):
  `_updater.setUpdateStatusListener(this);`
3. Call `checkForUpdates()`
4. The `onUpdateAvailable(CAutoUpdaterGithub::ChangeLog changelog)` callback will be called asynchronously (in the same thread that requested the check). If any updates were found, the `changelog` vector will be non-empty. You can use its items to retrieve the update details. If it's empty, it means no updates are available.
5. Call `downloadAndInstallUpdate()` to download the update and launch it.

# Building

Prerequisites:
* Qt 5.
* A compiler with C++11 support.

Build the project as you would any Qt-based static library.
