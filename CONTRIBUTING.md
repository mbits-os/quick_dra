# How to contribute

Thank you for your interest in contributing to this project.

_This guide is still a work in progress. Contributions are especially welcome to expand setup instructions, add coding guidelines, and refine the task list below—please open a PR or file an issue if something is missing or unclear._

In the meantime, please use the [issue templates](.github/ISSUE_TEMPLATE/) when reporting problems or requesting features and refer to [roadmap](docs/roadmap.md) for possible tasks to help with.

## Reporting bugs and requesting features

- Before opening a new issue, please search the existing issues to see if it has already been reported or discussed.
- When filing a bug, include:
  - What you were trying to do.
  - The exact command(s) you ran.
  - What you expected to happen.
  - What actually happened (including error messages and logs, if possible).
  - Information about your environment (OS, versions of relevant tools).
- For feature requests, describe the problem you are trying to solve and, if you have one, a rough idea of the desired solution.

## Contributing changes (pull requests)

- Fork the repository and create a topic branch from the main development branch.
- Make small, focused changes; avoid bundling unrelated fixes in a single pull request.
- This project uses _Conventional Commits_. Please refer to COMMITS file for overview of commit messages.
- If your change fixes an open issue, reference it in the pull request description (for example, `Fixes #123`).
- Update or add tests when you change behavior, and update documentation or comments if needed.
- Ensure the project builds and tests pass locally before opening the pull request.
- Be prepared to revise your pull request based on review feedback.

## Development setup

- Clone your fork and follow the setup instructions in the project [docs/building](docs/building.md) (dependencies, build steps, etc.).
- Use tooling, dependencies and helper scripts described in the building instructions.

## Testing

- _The test suite is non-existent at the moment. Take the following point with a grain of salt as long as this point is present._ Until this point is still here, add a short note in your pull request about how you verified your changes (for example, specific commands you ran or files you inspected).
- Run the test suite before pushing changes. Use `test` flow command, which combines Build and Test steps, so that binaries are re-build as needed before the tests are ran. The two commands below are equivalent:

  ```sh
  ./flow test --dbg
  ./flow run --dbg -s build,test
  ```

- Do not commit broken tests unless you clearly mark them as expected failures and there is prior agreement to do so.

## Code style and quality

- Follow the existing style of the surrounding code: indentation, naming, and file organization.
- This code is formatted using `clang-format-18` for C++ and `black`/`isort` for Python, run them before committing.
- CI will run `super-linter` on this code. It is advised to run it before committing through \
`./flow tools run-linter --log_level=NOTICE`.
- Keep functions and modules focused; prefer clear, readable code and comments where behavior might be non‑obvious.
- Avoid introducing new dependencies unless they are clearly justified.

Thanks,
Marcin
