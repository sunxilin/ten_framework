//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
pub mod found_result;
mod local;
mod remote;

use anyhow::{anyhow, Result};
use semver::{Version, VersionReq};
use tempfile::NamedTempFile;

use super::config::TmanConfig;
use found_result::FoundResult;
use ten_rust::pkg_info::{pkg_identity::PkgIdentity, PkgInfo};

pub struct SearchCriteria {
    pub version_req: VersionReq,
    // Future search fields can be added here.
}

pub async fn upload_package(
    tman_config: &TmanConfig,
    package_file_path: &str,
    pkg_info: &PkgInfo,
) -> Result<()> {
    let default_registry_url = tman_config
        .registry
        .get("default")
        .ok_or_else(|| anyhow!("Default registry not found"))?
        .index
        .clone();

    let parsed_registry_url = match url::Url::parse(&default_registry_url) {
        Ok(url) => url,
        Err(_) => return Err(anyhow!("Invalid URL: {}", default_registry_url)),
    };

    match parsed_registry_url.scheme() {
        "file" => {
            local::upload_package(
                &default_registry_url,
                package_file_path,
                pkg_info,
            )
            .await
        }
        "https" => {
            remote::upload_package(
                tman_config,
                &default_registry_url,
                package_file_path,
                pkg_info,
            )
            .await
        }
        _ => Err(anyhow!(
            "Unrecognized URL scheme: {}",
            parsed_registry_url.scheme()
        )),
    }
}

pub async fn get_package(
    tman_config: &TmanConfig,
    url: &str,
    temp_path: &mut NamedTempFile,
) -> Result<()> {
    let parsed_url =
        url::Url::parse(url).map_err(|_| anyhow!("Invalid URL: {}", url))?;

    match parsed_url.scheme() {
        "file" => local::get_package(tman_config, url, temp_path).await,
        "https" => remote::get_package(tman_config, url, temp_path).await,
        _ => Err(anyhow!("Failed to get package to any configured registry.")),
    }
}

pub async fn get_package_list(
    tman_config: &TmanConfig,
    pkg_identity: &PkgIdentity,
    criteria: &SearchCriteria,
) -> Result<Vec<FoundResult>> {
    // Retrieve the default registry URL
    let default_registry_url = tman_config
        .registry
        .get("default")
        .ok_or_else(|| anyhow!("Default registry not found"))?
        .index
        .clone();

    let parsed_registry_url = url::Url::parse(&default_registry_url)
        .map_err(|_| anyhow!("Invalid URL: {}", default_registry_url))?;

    let new_results = match parsed_registry_url.scheme() {
        "file" => {
            local::get_package_list(
                tman_config,
                &default_registry_url,
                pkg_identity,
                criteria,
            )
            .await?
        }
        "https" => {
            remote::get_package_list(
                tman_config,
                &default_registry_url,
                pkg_identity,
                criteria,
            )
            .await?
        }
        _ => {
            return Err(anyhow!(
                "Unsupported URL scheme: {}",
                parsed_registry_url.scheme()
            ));
        }
    };

    Ok(new_results)
}

pub async fn delete_package(
    tman_config: &TmanConfig,
    pkg_identity: &PkgIdentity,
    version: &Version,
    hash: &String,
) -> Result<()> {
    // Retrieve the default registry URL.
    let default_registry_url = tman_config
        .registry
        .get("default")
        .ok_or_else(|| anyhow!("Default registry not found"))?
        .index
        .clone();

    let parsed_registry_url = url::Url::parse(&default_registry_url)
        .map_err(|_| anyhow!("Invalid URL: {}", default_registry_url))?;

    match parsed_registry_url.scheme() {
        "file" => {
            local::delete_package(
                &default_registry_url,
                pkg_identity,
                version,
                hash,
            )
            .await
        }
        "https" => {
            remote::delete_package(
                tman_config,
                &default_registry_url,
                pkg_identity,
                version,
                hash,
            )
            .await
        }
        _ => Err(anyhow!(
            "Unsupported URL scheme: {}",
            parsed_registry_url.scheme()
        )),
    }
}
