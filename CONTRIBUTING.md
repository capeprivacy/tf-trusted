# Contribution Guide

The goal of TF Trusted is to make it easy for researchers and practitioners to experiment with various tricks and techniques running TensorFlow models inside secure enclaves. As such, contributions are more than welcome and we're always looking for use cases, feature ideas, or machine learning optimizations!

This document helps you get started on:

- [Submitting a pull request](#submitting-a-pull-request)
- [Reporting a bug](#reporting-a-bug)

# Submitting a Pull Request

To contribute, [fork](https://help.github.com/articles/fork-a-repo/) TF Trusted, commit your changes, and [open a pull request](https://help.github.com/articles/using-pull-requests/).

While you may be asked to make changes to your submission during the review process, we will work with you on this and suggest changes. Consider giving us [push rights to your branch](https://help.github.com/articles/allowing-changes-to-a-pull-request-branch-created-from-a-fork/) so we can potentially also help via commits.

## Commit history and merging

For the sake of transparency our key rule is to keep a logical and intelligible commit history, meaning anyone stepping through the commits on either the `master` branch or as part of a review should be able to easily follow the changes made and their potential implications.

To this end we ask all contributors to sanitize pull requests before submitting them as our preferred way of merging pull requests is rebasing.

Some guidelines:

- even simple code changes such as moving code around can obscure semantic changes, and in those case there should be two commits: one that e.g. only moves code (with a note of this in the commit description) and one that performs the semantic change

- progressions that have no logical justification for being split into several commits should be squeezed

- code does not have to compile or pass all tests at each commit, but leave a remark and a plan in the commit description so reviewers are aware and can plan accordingly

Useful tricks:

- [GitHub Desktop](https://desktop.github.com/) provides a useful interface for inspecting and committing code changes
- `git add -p` lets you leave out some changes in a file (GitHub Desktop can be used for this as well)
- `git commit --amend` allows you to add to the previous commit instead of creating a new one
- `git rebase -i HEAD~N` allows you to [squeeze and reorder commits](https://git-scm.com/book/en/v2/Git-Tools-Rewriting-History) and last `N` commits
- `git rebase master` to [pull in latest updates](https://git-scm.com/book/en/v2/Git-Branching-Rebasing) on `master`

## Before submitting for review

Make sure to give some context and overview in the body of your pull request to make it easier for reviewers to understand your changes. Ideally explain why your particular changes were made the way they are.

Importantly, use [keywords](https://help.github.com/en/articles/closing-issues-using-keywords) such as `Closes #<issue-number>` to indicate any issues or other pull requests related to your work.

# Reporting a Bug

Think you've found a bug? Let us know by opening an [issue in our tracker](https://github.com/mortendahl/tf-encrypted/issues) and apply the "bug" label!

## Security disclosures

If you encounter a security issue then please responsibly disclose it by reaching out to us via [contract@dropoutlabs.com](mailto:contant@dropoutlabs.com). We will work with you to mitigate the issue and responsibly disclose it to anyone using the project in a timely manner.
