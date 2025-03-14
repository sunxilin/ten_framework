//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
use std::{
    path::Path,
    sync::{Arc, RwLock},
};

use actix_web::{web, HttpResponse, Responder};
use serde::Serialize;

use crate::{
    constants::PROPERTY_JSON_FILENAME,
    dev_server::{
        get_all_pkgs::get_all_pkgs,
        response::{ApiResponse, ErrorResponse, Status},
        DevServerState,
    },
};
use ten_rust::pkg_info::pkg_type::PkgType;

#[derive(Serialize, Debug, PartialEq)]
struct DumpResponse {
    success: bool,
}

pub async fn dump_property(
    state: web::Data<Arc<RwLock<DevServerState>>>,
) -> impl Responder {
    let mut state = state.write().unwrap();

    // Fetch all packages if not already done.
    if let Err(err) = get_all_pkgs(&mut state) {
        let error_response = ErrorResponse {
            status: Status::Fail,
            message: format!("Error fetching packages: {}", err),
            error: None,
        };
        return HttpResponse::NotFound().json(error_response);
    }

    let property_file_path = Path::join(
        Path::new(state.base_dir.as_ref().unwrap()),
        PROPERTY_JSON_FILENAME,
    );

    if let Some(pkgs) = &mut state.all_pkgs {
        if let Some(app_pkg) = pkgs
            .iter_mut()
            .find(|pkg| pkg.pkg_identity.pkg_type == PkgType::App)
        {
            let response = ApiResponse {
                status: Status::Ok,
                data: DumpResponse { success: true },
                meta: None,
            };

            if app_pkg.property.is_none() {
                return HttpResponse::Ok().json(response);
            }

            let app_prop = app_pkg.property.as_ref().unwrap();
            if let Err(err) =
                app_prop.dump_property_to_file(&property_file_path)
            {
                let error_response = ErrorResponse {
                    status: Status::Fail,
                    message: format!(
                        "Failed to dump new content to property.json: {}",
                        err
                    ),
                    error: None,
                };
                return HttpResponse::NotFound().json(error_response);
            }

            HttpResponse::Ok().json(response)
        } else {
            let error_response = ErrorResponse {
                status: Status::Fail,
                message: "Failed to find app package.".to_string(),
                error: None,
            };
            HttpResponse::NotFound().json(error_response)
        }
    } else {
        let error_response = ErrorResponse {
            status: Status::Fail,
            message: "Package information is missing".to_string(),
            error: None,
        };
        HttpResponse::NotFound().json(error_response)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{
        config::TmanConfig,
        dev_server::{
            graphs::update::update_graph, mock::tests::inject_all_pkgs_for_mock,
        },
        utils::read_file_to_string,
    };
    use actix_web::{test, App};
    use serde_json::Value;
    use std::{env, fs};
    use ten_rust::pkg_info::property::parse_property_from_file;

    #[actix_web::test]
    async fn test_dump_property_success() {
        // Get current working directory when testing.
        let exe_path =
            env::current_exe().expect("Failed to get the executable path");
        let test_dir = exe_path
            .parent()
            .expect("Failed to get the parent directory");

        env::set_current_dir(test_dir)
            .expect("Failed to set current directory");

        let test_data_dir = test_dir.join("test_data");
        fs::create_dir_all(&test_data_dir)
            .expect("Failed to create test_data directory");

        let mut dev_server_state = DevServerState {
            base_dir: Some(test_data_dir.to_string_lossy().to_string()),
            all_pkgs: None,
            tman_config: TmanConfig::default(),
        };

        let all_pkgs_json = vec![
            (
                include_str!(
                    "../test_data/large_response_source_app_manifest.json"
                )
                .to_string(),
                include_str!(
                    "../test_data/large_response_source_app_property.json"
                )
                .to_string(),
            ),
            (
                include_str!("../test_data/extension_addon_1_manifest.json")
                    .to_string(),
                "{}".to_string(),
            ),
            (
                include_str!("../test_data/extension_addon_2_manifest.json")
                    .to_string(),
                "{}".to_string(),
            ),
            (
                include_str!("../test_data/extension_addon_3_manifest.json")
                    .to_string(),
                "{}".to_string(),
            ),
        ];

        let inject_ret =
            inject_all_pkgs_for_mock(&mut dev_server_state, all_pkgs_json);
        assert!(inject_ret.is_ok());

        let dev_server_state = Arc::new(RwLock::new(dev_server_state));

        let app = test::init_service(
            App::new()
                .app_data(web::Data::new(dev_server_state.clone()))
                .route(
                    "/api/dev-server/v1/graphs/{graph_name}",
                    web::put().to(update_graph),
                )
                .route(
                    "/api/dev-server/v1/property",
                    web::put().to(dump_property),
                ),
        )
        .await;

        let input_data: Value = serde_json::from_str(include_str!(
            "test_data/test_dump_property_success_input_data.json"
        ))
        .unwrap();

        let req = test::TestRequest::put()
            .uri("/api/dev-server/v1/graphs/0")
            .set_json(input_data)
            .to_request();
        let resp = test::call_service(&app, req).await;

        assert!(resp.status().is_success());

        // Create a fake property.json for testing.
        let property_file_path = test_data_dir.join(PROPERTY_JSON_FILENAME);

        let req = test::TestRequest::put()
            .uri("/api/dev-server/v1/property")
            .to_request();
        let resp = test::call_service(&app, req).await;

        assert!(resp.status().is_success());

        assert!(property_file_path.exists());

        let property = parse_property_from_file(&property_file_path).unwrap();
        let predefined_graphs =
            property._ten.unwrap().predefined_graphs.unwrap();

        let property_predefined_graph =
            predefined_graphs.iter().find(|g| g.name == "0").unwrap();

        assert_eq!(property_predefined_graph.auto_start, Some(false));
        assert_eq!(property_predefined_graph.graph.nodes.len(), 2);
        assert_eq!(
            property_predefined_graph
                .graph
                .connections
                .as_ref()
                .unwrap()
                .len(),
            1
        );

        let new_property_str =
            read_file_to_string(property_file_path.clone()).unwrap();
        let new_property_json: serde_json::Value =
            serde_json::from_str(&new_property_str).unwrap();
        assert!(new_property_json.is_object());

        fs::remove_file(property_file_path)
            .expect("Failed to remove property.json file");
    }
}
