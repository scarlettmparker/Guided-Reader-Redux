import { RouteObject, useRoutes } from "react-router-dom";
import Index from "~/routes/index";

const routes: RouteObject[] = [
  {
    path: "/",
    element: <Index />,
  },
];

export const Router = () => {
  return useRoutes(routes);
};
