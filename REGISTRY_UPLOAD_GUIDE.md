# ESP-IDF Button Component - Registry Upload Guide

## Steps to Upload Your Component to ESP-IDF Component Registry

### 1. **Prepare Your GitHub Repository**

First, you need to have this component in a public GitHub repository:

#### Create a GitHub Repository
1. Go to [github.com](https://github.com) and sign in
2. Click the **+** icon in the top-right and select **New repository**
3. Name it: `button` (or similar)
4. Add description: "Interrupt-driven button handler for ESP32"
5. Make it **Public** (required for component registry)
6. Initialize with README (we'll overwrite it)
7. Choose MIT License

#### Push Your Component to GitHub

```bash
# Navigate to your component directory
cd e:\MCU\vsCode\ESP-IDF\Lib\buttons

# Initialize git if not already done
git init

# Add all files
git add .

# Commit your changes
git commit -m "Initial commit: Button component for ESP32"

# Add remote repository (replace YOUR_USERNAME and YOUR_REPO)
git remote add origin https://github.com/YOUR_USERNAME/button.git

# Push to GitHub
git branch -M main
git push -u origin main
```

### 2. **Update idf_component.yml with GitHub Details**

Edit the `idf_component.yml` file and replace the placeholder URLs:

**BEFORE:**
```yaml
url: "https://github.com/YOUR_USERNAME/button"
documentation: "https://github.com/YOUR_USERNAME/button/blob/main/README.md"
repository: "https://github.com/YOUR_USERNAME/button"
issues: "https://github.com/YOUR_USERNAME/button/issues"
```

**AFTER (Example):**
```yaml
url: "https://github.com/ashutosh/button"
documentation: "https://github.com/ashutosh/button/blob/main/README.md"
repository: "https://github.com/ashutosh/button"
issues: "https://github.com/ashutosh/button/issues"
```

### 3. **Create a Release on GitHub**

This is important for the registry to recognize your component:

```bash
# From your local repository
git tag v1.0.0
git push origin v1.0.0
```

Or via GitHub UI:
1. Go to your repository page
2. Click **Releases** on the right sidebar
3. Click **Create a new release**
4. Set tag to `v1.0.0`
5. Set release title to `v1.0.0 - Initial Release`
6. Add release notes describing the features
7. Click **Publish release**

### 4. **Verify Your Component Structure**

Ensure your component has ALL these files:

```
button/
├── button.cpp
├── button.h (or include/button.h)
├── CMakeLists.txt
├── idf_component.yml      ✓ REQUIRED
├── README.md              ✓ REQUIRED
├── LICENSE                ✓ REQUIRED
└── include/
    └── button.h
```

**Checklist:**
- [ ] `idf_component.yml` is present and properly formatted
- [ ] `README.md` has clear documentation
- [ ] `LICENSE` file exists (MIT is recommended)
- [ ] `CMakeLists.txt` is correct
- [ ] All files are committed to Git
- [ ] GitHub repository is PUBLIC

### 5. **Submit to ESP-IDF Component Registry**

#### Option A: Using the Registry Website (Recommended)

1. Visit: https://components.espressif.com/
2. Click **Submit Component** or **Upload Component**
3. Sign in with your GitHub account
4. Select your repository: `YOUR_USERNAME/button`
5. Choose the release version (e.g., `v1.0.0`)
6. Fill in component details:
   - **Component Name**: `button`
   - **Description**: "Interrupt-driven button handler for ESP32 with debouncing and event callbacks"
   - **Version**: `1.0.0`
   - **License**: `MIT`
7. Review the preview
8. Click **Submit**

#### Option B: Using idf.py Command

```bash
# Configure your GitHub credentials
idf.py add-component-registry --registry-url https://github.com/YOUR_USERNAME/button

# Or directly register component
idf.py component-manager registry add --registry-name button
```

### 6. **Verify Your Component is Published**

1. Visit https://components.espressif.com/
2. Search for "button"
3. Your component should appear in the results

### 7. **Make Your Component Installable**

Once published, users can install it with:

```bash
# In their ESP-IDF project
idf.py add-dependency button
```

Or manually edit `idf_component.yml` in their project:
```yaml
dependencies:
  button: "^1.0.0"
```

### 8. **Creating Updates and New Releases**

To release a new version:

```bash
# Make your changes
git add .
git commit -m "v1.1.0: Added feature X"

# Create a new tag
git tag v1.1.0
git push origin v1.1.0

# Then create a GitHub release for the tag
```

The registry will automatically recognize the new version.

---

## Important Notes

### idf_component.yml Requirements

Your `idf_component.yml` must include:

```yaml
version: "1.0.0"                          # REQUIRED - Follow semantic versioning
description: "..."                         # REQUIRED
url: "https://github.com/..."             # REQUIRED - Public GitHub URL
documentation: "https://github.com/.../README.md"  # REQUIRED
repository: "https://github.com/..."      # REQUIRED
license: "MIT"                            # RECOMMENDED
maintainers:                              # RECOMMENDED
  - name: "Your Name"
    email: "your.email@example.com"
dependencies:                             # OPTIONAL
  esp-idf: ">=4.4"
tags:                                     # OPTIONAL - for discoverability
  - "button"
  - "input"
  - "gpio"
```

### Semantic Versioning

Follow semantic versioning: `MAJOR.MINOR.PATCH`
- `1.0.0` - First release
- `1.1.0` - Added features (backward compatible)
- `1.1.1` - Bug fixes (backward compatible)
- `2.0.0` - Breaking changes

### README Requirements

Your README should include:
- ✓ Clear description
- ✓ Features list
- ✓ Installation instructions
- ✓ Basic usage example
- ✓ API reference
- ✓ Hardware connection diagram
- ✓ License information
- ✓ Troubleshooting

### GitHub Repository Requirements

- ✓ **Public** (not private)
- ✓ Proper `.gitignore` (optional but recommended)
- ✓ At least one release tag (e.g., `v1.0.0`)
- ✓ All files committed to main branch

---

## Troubleshooting

### Component not appearing in registry after submission
- Wait 15-30 minutes for the registry to index your component
- Check that your GitHub repository is PUBLIC
- Verify `idf_component.yml` is valid YAML
- Ensure version in `idf_component.yml` matches your Git tag

### Users can't install your component
- Verify the component name in `idf_component.yml` matches your repository
- Check that the GitHub repository URL is correct
- Ensure the release tag exists (e.g., `v1.0.0`)

### idf_component.yml validation errors
- Use an online YAML validator: https://www.yamllint.com/
- Ensure proper indentation (spaces, not tabs)
- Check for correct data types (strings in quotes, versions as strings)

---

## Getting Help

- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **Component Manager Documentation**: https://github.com/espressif/idf-component-manager
- **Registry Issues**: https://github.com/espressif/component-registry-issues

---

## Example Final Structure

```
button/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── button.cpp
├── idf_component.yml
├── .git/
├── .github/
│   └── workflows/          (optional - for CI/CD)
└── include/
    └── button.h
```

---

## Next Steps After Upload

1. Monitor registry for installations
2. Collect user feedback and issues
3. Create issues/discussions in your GitHub repository
4. Update your component with improvements
5. Release new versions as needed
6. Consider adding GitHub Actions for CI/CD

---

**Good luck with your component submission! 🚀**
