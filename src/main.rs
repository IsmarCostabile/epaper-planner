use axum::{
    routing::get,
    Router,
    extract::State,
    response::{Html, IntoResponse},
};
use std::sync::Arc;
use tera::Tera;
use tokio::net::TcpListener;
use std::ffi::CString;

async fn index_handler(State(tera): State<Arc<Tera>>) -> impl IntoResponse {
    let rendered = tera.render("index.html", &tera::Context::new())
        .unwrap_or_else(|_| "<h1>Error rendering page</h1>".to_string());
    Html(rendered)
}

// Declare C library functions
extern "C" {
    fn printf(format: *const std::os::raw::c_char, ...) -> std::os::raw::c_int;
    fn puts(s: *const std::os::raw::c_char) -> std::os::raw::c_int;
}

async fn button_handler() -> impl IntoResponse {
    let msg = CString::new("Button pressed from C!\n").unwrap();
    unsafe {
        puts(msg.as_ptr()); // or use printf
    }
    Html("<h2>Button was pressed! Check console output.</h2>")
}

#[tokio::main]
async fn main() {
    let tera = Tera::new("templates/**/*").unwrap();
    let shared_state = Arc::new(tera);
    
    let app = Router::new()
        .route("/", get(index_handler))
        .route("/button", get(button_handler))
        .with_state(shared_state);
    
    let listener = TcpListener::bind("0.0.0.0:3000").await.unwrap();
    println!("Server running on http://0.0.0.0:3000");
    
    axum::serve(listener, app).await.unwrap();
}