import { RouteObject, useRoutes } from "react-router-dom";
import Index from "~/routes/index";
import Login from "~/routes/login";

const routes: RouteObject[] = [
  {
    path: "/",
    element: <Index />,
  },
  {
    path: '/login',
    element: <Login />
  }
];

export const Router = () => {
  return useRoutes(routes);
};
