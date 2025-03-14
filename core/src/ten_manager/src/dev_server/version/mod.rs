//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
use std::sync::{Arc, RwLock};

use actix_web::{web, HttpResponse, Responder};
use serde::{Deserialize, Serialize};

use super::{
    response::{ApiResponse, Status},
    DevServerState,
};
use crate::version::VERSION;

#[derive(Serialize, Deserialize, PartialEq, Debug)]
struct DevServerVersion {
    version: String,
}

pub async fn get_version(
    _state: web::Data<Arc<RwLock<DevServerState>>>,
) -> impl Responder {
    let version_info = DevServerVersion {
        version: VERSION.to_string(),
    };

    let response = ApiResponse {
        status: Status::Ok,
        data: version_info,
        meta: None,
    };

    HttpResponse::Ok().json(response)
}

#[cfg(test)]
mod tests {
    use crate::config::TmanConfig;

    use super::*;
    use actix_web::{http::StatusCode, test, App};

    #[actix_web::test]
    async fn test_get_version() {
        // Initialize the DevServerState.
        let state = web::Data::new(Arc::new(RwLock::new(DevServerState {
            base_dir: None,
            all_pkgs: None,
            tman_config: TmanConfig::default(),
        })));

        // Create the App with the routes configured.
        let app =
            test::init_service(App::new().app_data(state.clone()).route(
                "/api/dev-server/v1/version",
                web::get().to(get_version),
            ))
            .await;

        // Send a request to the version endpoint.
        let req = test::TestRequest::get()
            .uri("/api/dev-server/v1/version")
            .to_request();
        let resp = test::call_service(&app, req).await;

        // Check that the response status is 200 OK.
        assert_eq!(resp.status(), StatusCode::OK);

        assert!(resp.status().is_success());

        let body = test::read_body(resp).await;
        let body_str = std::str::from_utf8(&body).unwrap();

        let version: ApiResponse<DevServerVersion> =
            serde_json::from_str(body_str).unwrap();

        // Create the expected Version struct
        let expected_version = DevServerVersion {
            version: VERSION.to_string(),
        };

        // Compare the actual Version struct with the expected one
        assert_eq!(version.data, expected_version);

        let json: serde_json::Value = serde_json::from_str(body_str).unwrap();
        let pretty_json = serde_json::to_string_pretty(&json).unwrap();
        println!("Response body: {}", pretty_json);
    }
}
