//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#[cfg(test)]
pub mod tests {
    use std::str::FromStr;

    use anyhow::Result;

    use crate::dev_server::DevServerState;
    use ten_rust::pkg_info::PkgInfo;
    use ten_rust::pkg_info::{manifest::Manifest, property::Property};

    pub fn inject_all_pkgs_for_mock(
        state: &mut DevServerState,
        all_pkgs_manifest_json: Vec<(String, String)>,
    ) -> Result<()> {
        if state.all_pkgs.is_some() {
            return Err(anyhow::anyhow!("The all_pkgs field is already set"));
        }

        let mut all_pkgs: Vec<PkgInfo> = Vec::new();
        for metadata_json in all_pkgs_manifest_json {
            let manifest = Manifest::from_str(&metadata_json.0)?;
            let property = Property::from_str(&metadata_json.1)?;

            let pkg_info = PkgInfo::from_metadata(&manifest, &Some(property))?;
            all_pkgs.push(pkg_info);
        }

        state.all_pkgs = Some(all_pkgs);

        Ok(())
    }
}
