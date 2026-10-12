# VS Code C/C++ Environment Sync Guide (Linux)

Settings Sync synchronizes editor settings and extensions, while Git synchronizes source code and project configuration. Prepare the development tools on the Linux host where you actually compile and debug.

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

```bash
cd "<project-directory>"

git init
git add .
git diff --cached --stat
git commit -m "Save project configuration"
git remote add origin "<repository-url>"
git push -u origin HEAD
```

**If the project already has a remote repository:**

```bash
cd "<project-directory>"

git add .
git diff --cached --stat
git commit -m "Update project configuration"
git push
```

## 2. Target environment: restore synchronized configuration

Choose local Linux or remote Ubuntu to match your development workflow.

### Option A: local Linux

Prerequisites: VS Code, Git, CMake, the compiler, debugger, and clangd are ready on the Linux computer.

**Restore editor configuration — VS Code Command Palette:**

```text
Settings Sync: Turn On
```

Use the same account and development Profile as on the original computer.

**First checkout — Linux terminal:**

```bash
git clone "<repository-url>" "<local-project-directory>"
cd "<local-project-directory>"
code .
```

**Update an existing checkout — Linux terminal:**

```bash
cd "<local-project-directory>"
git pull
code .
```

### Option B: remote Ubuntu

Prerequisites: Git, CMake, the compiler, debugger, and clangd are ready on the Ubuntu host, and Remote-SSH is installed in your local VS Code.

**Restore local editor configuration — local VS Code Command Palette:**

```text
Settings Sync: Turn On
```

**Connect to Ubuntu — local VS Code Command Palette:**

```text
Remote-SSH: Connect to Host...
```

**First checkout — remote terminal after connecting:**

```bash
git clone "<repository-url>" "<remote-project-directory>"
```

**Update an existing checkout — remote terminal after connecting:**

```bash
cd "<remote-project-directory>"
git pull
```

**Open the remote project — Command Palette in the remote window:**

```text
File: Open Folder...
```

Open the `<remote-project-directory>` used above.

**Install remote development extensions — Command Palette in the remote window:**

```text
Extensions: Install Extensions
```

Install the extensions on the current SSH host. Settings Sync does not automatically restore remote extensions, and the Ubuntu host does not need a separate sync-account sign-in.

### Final step for both options: reconfigure and build

In the VS Code window containing the project, run these commands in order:

```text
CMake: Select Configure Preset
CMake: Configure
CMake: Build
```

Select an existing Linux configuration provided by the project. Regenerate the build directory and compilation database on the current Linux host using the project's existing approach.

To refresh clangd, run:

```text
clangd: Restart language server
```

## 3. Everyday synchronization

**In the current environment: commit and push changes.**

```bash
cd "<project-directory>"

git add .
git diff --cached --stat
git commit -m "Update project"
git push
```

**On other computers or remote hosts: get the latest commits.**

```bash
cd "<project-directory>"
git pull
```

When using Remote-SSH, run Git commands in the remote terminal.

## 4. New projects: reuse a template

Prerequisites: a template repository is available, and you have created an empty remote repository for the new project.

```bash
git clone "<template-repository-url>" "<new-project-directory>"
cd "<new-project-directory>"

git remote set-url origin "<new-project-repository-url>"
git push -u origin HEAD
```

Open the project on local Linux:

```bash
code .
```

With Remote-SSH, open the project using the Command Palette in the remote window:

```text
File: Open Folder...
```

This method retains the template repository's files and commit history. After opening the new project, reconfigure and build using the commands in section 2.
