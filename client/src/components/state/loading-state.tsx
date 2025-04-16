import { Component, JSX } from "solid-js";

interface LoadingStateProps {
  children: JSX.Element | string;
};

const LoadingState: Component<LoadingStateProps> = (props) => {
  const { children } = props;

  return (
    <span>
      {children}
    </span>
  );
};

export default LoadingState;