import { RouteObject, useRoutes } from "react-router-dom";
import Index from "~/routes/index";
import Login from "~/routes/login";
import NotFound from "~/routes/not-found/not-found";

const routes: RouteObject[] = [
  {
    path: "/",
    element: <Index />,
  },
  {
    path: '/login',
    element: <Login />
  },
  // Catch-all 404 route
  {
    path: '*',
    element: <NotFound />
  }
];

export const Router = () => {
  return useRoutes(routes);
};
