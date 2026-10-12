# VS Code C/C++ Environment Sync Guide (Windows)

Settings Sync synchronizes editor settings and extensions, while Git synchronizes source code and project configuration. Prepare the development tools separately on each computer.

The `<...>` values in the following commands are placeholders. Replace them with your actual values.

## 1. Original computer: prepare for synchronization

### 1. Enable VS Code Settings Sync

Run the following command in the VS Code Command Palette:

```text
Settings Sync: Turn On
```

Sign in with a Microsoft or GitHub account.

Synchronize settings, keyboard shortcuts, extensions, user snippets, and Profiles.

### 2. Save project configuration

The project's `.gitignore` should already exclude build directories, generated `compile_commands.json` files, `CMakeUserPresets.json`, and other machine-specific files.

Use Git to save source code, CMake configuration, and shareable configuration in `.vscode/`.

**If the project does not have a Git repository yet:**

```powershell
Set-Location "<project-directory>"

git init
git add .
git diff --cached --stat
git commit -m "Save project configuration"
git remote add origin "<repository-url>"
git push -u origin HEAD
```

**If the project already has a remote repository:**

```powershell
Set-Location "<project-directory>"

git add .
git diff --cached --stat
git commit -m "Update project configuration"
git push
```

## 2. New computer: restore synchronized configuration

Prerequisites: VS Code, Git, CMake, and the compiler, debugger, and clangd used by the project are ready.

### 1. Restore editor configuration

Run the following command in the VS Code Command Palette:

```text
Settings Sync: Turn On
```

Use the same account and development Profile as on the original computer.

### 2. Get the project

**First checkout:**

```powershell
git clone "<repository-url>" "<local-project-directory>"
Set-Location "<local-project-directory>"
code .
```

**Update an existing checkout:**

```powershell
Set-Location "<local-project-directory>"
git pull
code .
```

### 3. Reconfigure and build

In the VS Code window where the project is open, run these commands in the Command Palette in order:

```text
CMake: Select Configure Preset
CMake: Configure
CMake: Build
```

Select an existing Windows configuration provided by the project. Regenerate the build directory and compilation database on the new computer using the project's existing approach.

To refresh clangd, run:

```text
clangd: Restart language server
```

## 3. Everyday synchronization

**On the current computer: commit and push changes.**

```powershell
Set-Location "<project-directory>"

git add .
git diff --cached --stat
git commit -m "Update project"
git push
```

**On other computers: get the latest commits.**

```powershell
Set-Location "<project-directory>"
git pull
```

Settings Sync automatically synchronizes VS Code user settings. Use the Git commands above to synchronize project files.

## 4. New projects: reuse a template

Prerequisites: a template repository is available, and you have created an empty remote repository for the new project.

```powershell
git clone "<template-repository-url>" "<new-project-directory>"
Set-Location "<new-project-directory>"

git remote set-url origin "<new-project-repository-url>"
git push -u origin HEAD

code .
```

This method retains the template repository's files and commit history. After opening the new project, reconfigure and build using the commands in section 2.
