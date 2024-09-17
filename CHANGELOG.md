# Changelog

## [Unreleased](https://github.com/btschwertfeger/BiasAdjustCXX/tree/HEAD)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.9.3...HEAD)

**Fixed bugs:**

- The -v option is used twice, for `--variable` and `--version` [\#39](https://github.com/btschwertfeger/BiasAdjustCXX/issues/39)

## [v1.9.3](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.9.3) (2024-09-09)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.9.2...v1.9.3)

**Fixed bugs:**

- The -v option is used twice, for `--variable` and `--version` [\#39](https://github.com/btschwertfeger/BiasAdjustCXX/issues/39)
- Resolve "The -v option is used twice, for `--variable` and `--version`" [\#40](https://github.com/btschwertfeger/BiasAdjustCXX/pull/40) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.9.2](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.9.2) (2024-02-02)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.9.1...v1.9.2)

**Merged pull requests:**

- Update documentation, readme and update workflows [\#37](https://github.com/btschwertfeger/BiasAdjustCXX/pull/37) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.9.1](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.9.1) (2023-08-09)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.9.0...v1.9.1)

**Fixed bugs:**

- The tool runs into segmentation faults when the input data contains NAN values [\#32](https://github.com/btschwertfeger/BiasAdjustCXX/issues/32)
- Fix segmentation fault for time series including NaN values [\#31](https://github.com/btschwertfeger/BiasAdjustCXX/pull/31) ([btschwertfeger](https://github.com/btschwertfeger))

**Merged pull requests:**

- Readme: add table of content; fix wrong section numbering; fix badge [\#30](https://github.com/btschwertfeger/BiasAdjustCXX/pull/30) ([btschwertfeger](https://github.com/btschwertfeger))
- Add hints for NetCDFcxx not found error; fix typos [\#29](https://github.com/btschwertfeger/BiasAdjustCXX/pull/29) ([btschwertfeger](https://github.com/btschwertfeger))
- Add release workflow [\#27](https://github.com/btschwertfeger/BiasAdjustCXX/pull/27) ([btschwertfeger](https://github.com/btschwertfeger))
- Clarified difference between stochastic and non-stochastic climate variables in doc and readme [\#26](https://github.com/btschwertfeger/BiasAdjustCXX/pull/26) ([btschwertfeger](https://github.com/btschwertfeger))
- Added more checks during testing [\#25](https://github.com/btschwertfeger/BiasAdjustCXX/pull/25) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.9.0](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.9.0) (2023-04-20)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.8.1...v1.9.0)

**Breaking changes:**

- Change the `--n_processes` option to `--processes`  [\#17](https://github.com/btschwertfeger/BiasAdjustCXX/issues/17)

**Implemented enhancements:**

- Create a Documentation using sphinx [\#16](https://github.com/btschwertfeger/BiasAdjustCXX/issues/16)
- Create install and uninstall target / command [\#12](https://github.com/btschwertfeger/BiasAdjustCXX/issues/12)
- Add unit tests [\#8](https://github.com/btschwertfeger/BiasAdjustCXX/issues/8)
- Change Docker base image from Ubuntu22.04 to Alpine3.17 [\#7](https://github.com/btschwertfeger/BiasAdjustCXX/issues/7)
- Add a sphinx-based documentation [\#23](https://github.com/btschwertfeger/BiasAdjustCXX/pull/23) ([btschwertfeger](https://github.com/btschwertfeger))

**Fixed bugs:**

- Quantile Delta Mapping return nan values because of zero-devision error [\#19](https://github.com/btschwertfeger/BiasAdjustCXX/issues/19)
- Added a function to ensure the zero-devision and devision by zero [\#22](https://github.com/btschwertfeger/BiasAdjustCXX/pull/22) ([btschwertfeger](https://github.com/btschwertfeger))

**Closed issues:**

- Raise exception when `--max-scaling-factor` is set to `0`  [\#18](https://github.com/btschwertfeger/BiasAdjustCXX/issues/18)
- Create a basic CI [\#15](https://github.com/btschwertfeger/BiasAdjustCXX/issues/15)
- Add a Changelog [\#14](https://github.com/btschwertfeger/BiasAdjustCXX/issues/14)
- Add the related publication as documentation   [\#11](https://github.com/btschwertfeger/BiasAdjustCXX/issues/11)

**Merged pull requests:**

- Prepare the release [\#24](https://github.com/btschwertfeger/BiasAdjustCXX/pull/24) ([btschwertfeger](https://github.com/btschwertfeger))
- Add a Changelog [\#21](https://github.com/btschwertfeger/BiasAdjustCXX/pull/21) ([btschwertfeger](https://github.com/btschwertfeger))
- Adjusted the issue template [\#20](https://github.com/btschwertfeger/BiasAdjustCXX/pull/20) ([btschwertfeger](https://github.com/btschwertfeger))
- Added the DOI of the publication at SoftwareX in the README [\#13](https://github.com/btschwertfeger/BiasAdjustCXX/pull/13) ([btschwertfeger](https://github.com/btschwertfeger))
- Add Unit Tests; Simplify the Installation; Create the CI [\#9](https://github.com/btschwertfeger/BiasAdjustCXX/pull/9) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.8.1](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.8.1) (2023-03-11)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.8...v1.8.1)

**Merged pull requests:**

- Add docker image [\#6](https://github.com/btschwertfeger/BiasAdjustCXX/pull/6) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.8](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.8) (2023-01-10)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.7...v1.8)

## [v1.7](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.7) (2022-11-16)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.6.1...v1.7)

**Merged pull requests:**

- Add interval scaling as default [\#5](https://github.com/btschwertfeger/BiasAdjustCXX/pull/5) ([btschwertfeger](https://github.com/btschwertfeger))
- Add interval scaling [\#4](https://github.com/btschwertfeger/BiasAdjustCXX/pull/4) ([btschwertfeger](https://github.com/btschwertfeger))
- added max-scaling-factor parameter to avoid unrealistic results [\#3](https://github.com/btschwertfeger/BiasAdjustCXX/pull/3) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.6.1](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.6.1) (2022-11-02)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.6...v1.6.1)

**Merged pull requests:**

- Add testing program [\#2](https://github.com/btschwertfeger/BiasAdjustCXX/pull/2) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.6](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.6) (2022-10-31)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.5...v1.6)

## [v1.5](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.5) (2022-10-28)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/v1.0...v1.5)

**Merged pull requests:**

- Vectorized computation [\#1](https://github.com/btschwertfeger/BiasAdjustCXX/pull/1) ([btschwertfeger](https://github.com/btschwertfeger))

## [v1.0](https://github.com/btschwertfeger/BiasAdjustCXX/tree/v1.0) (2022-10-13)

[Full Changelog](https://github.com/btschwertfeger/BiasAdjustCXX/compare/d9cd18f57ada1713f00f3600aed5b9d2f38f4d45...v1.0)



\* *This Changelog was automatically generated by [github_changelog_generator](https://github.com/github-changelog-generator/github-changelog-generator)*
