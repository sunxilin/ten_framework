//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
use std::collections::HashMap;

use serde::Deserialize;

use super::graphs::nodes::{
    DevServerApiCmdLike, DevServerApiDataLike, DevServerPropertyAttributes,
};
use ten_rust::pkg_info::api::{
    PkgApiCmdLike, PkgApiDataLike, PkgPropertyAttributes,
};

pub fn get_dev_server_property_hashmap_from_pkg(
    items: HashMap<String, PkgPropertyAttributes>,
) -> HashMap<String, DevServerPropertyAttributes> {
    items.into_iter().map(|(k, v)| (k, v.into())).collect()
}

pub fn get_dev_server_api_cmd_likes_from_pkg(
    items: Vec<PkgApiCmdLike>,
) -> Vec<DevServerApiCmdLike> {
    items.into_iter().map(|v| v.into()).collect()
}

pub fn get_dev_server_api_data_likes_from_pkg(
    items: Vec<PkgApiDataLike>,
) -> Vec<DevServerApiDataLike> {
    items.into_iter().map(|v| v.into()).collect()
}

#[derive(Deserialize)]
pub struct CheckTypeQuery {
    #[serde(rename = "type")]
    pub check_type: String,
}
