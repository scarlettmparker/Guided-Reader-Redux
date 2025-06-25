import { RouteObject, useRoutes } from "react-router-dom";
import Index from "~/routes/index";
import Login from "~/routes/login";
import NotFound from "~/routes/not-found/not-found";
import User from "./routes/user";

const routes: RouteObject[] = [
  {
    path: "/",
    element: <Index />,
  },
  {
    path: "/login",
    element: <Login />,
  },
  {
    path: "/user/:userId",
    element: <User />,
  },
  {
    path: "*",
    element: <NotFound />,
  },
];

export const Router = () => {
  return useRoutes(routes);
};
