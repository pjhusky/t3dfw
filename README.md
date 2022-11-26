# Tiny 3D Framework

Tiny framework meant to be included directly "alongside" your application. 

# Quick Start

In your new application, run
    
	git submodule add https://github.com/pjhusky/t3dfw.git t3dfw

while in the project's root directory. This adds the framework to the folder t3dfw inside your project's root folder. Note that for this to work, your appliation must already be a valid git repository, e.g., you must run `git init` or clone another git repo beforehand. 

# Keeping t3dfw up to date

Later on, in your application you can update to the latest remote submodule commits by running
    
    git submodule update --remote --merge
    
# Pushing changes done to one of the submodules

Navigate to submodule folder and run
	
	git checkout main

Now you can basically interact with git as if this submodule was a normal repo. Just make sure to not forget committing & pushing the changed submodule from the main/root git repo as well.

### Note
If `git checkout main` was not performed, upon running `git status` you would see

	<ProjectPath>\t3dfw\src\fileLoaders>git status
	HEAD detached at a56a295

After running `git checkout main`:

	aProjectPath>\t3dfw\src\fileLoaders>git checkout main
	Switched to branch 'main'
	M       netBpmIO.h
	...

Now `git status` reports:

	C:\FM-koop\DVR-fetch-test\DVR-t3dfw\t3dfw\src\fileLoaders>git  status
	On branch main
	Your branch is up to date with 'origin/main'.

