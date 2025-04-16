import { Component, JSX } from "solid-js";

interface ErrorStateProps {
  children: JSX.Element | string;
};

const ErrorState: Component<ErrorStateProps> = (props) => {
  const { children } = props;

  return (
    <span>
      {children}
    </span>
  );
};

export default ErrorState;