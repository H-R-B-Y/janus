## TODO

- Config file for specifying the interfaces to scan, for username and password display, and other settings.
- Generic interface to support more than just the e-paper display, such as LCD or other serial display types. Output should be generic, terminal mode should just wrap stdout, e-paper mode should wrap the display functions, etc. need to come up with a good way to abstract this without losing the control features enabled by the direct display API.
- Figure out why some logging is missing from the journal when running as a systemd service
- Figure out why the signal handler is not working as expected when running as a service on RPI.
- Cleanup the stdin handler (refactor, split up the code.)
- Proper daemonization, right now when starting the service systemctl hangs for some reason, not sure why.
- Startup routine (Logo) and logging to the display.
- Ensure logging is kept to a minimum to avoid constant journal writes when running as a service.
- Remove over-reliance on macro blocks, move to interface functions where possible.