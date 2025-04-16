import { Component, JSX } from "solid-js";
import { LoaderCircle } from "~/components/lucide";

import styles from "./styles/loading-state.module.css";

interface LoadingStateProps {
  class?: string;
  children: JSX.Element | string;
};

const LoadingState: Component<LoadingStateProps> = (props) => {
  const { class: class_ = "", children } = props;

  return (
    <span class={`${styles.loading_state} ${class_}`}>
      <LoaderCircle /> {children}
    </span>
  );
};

export default LoadingState;