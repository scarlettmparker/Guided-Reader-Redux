import { render } from "solid-js/web";
import { Router, Route } from "@solidjs/router";
import { MetaProvider } from "@solidjs/meta";

import "./index.css";

import Index from "./routes/index";

render(
  () => (
    <MetaProvider>
      <Router>
        <Route path="/" component={Index} />
      </Router>
    </MetaProvider>
  ),
  document.getElementById("root")!
);
